/*
*   You can create your own certificate with below command:
*
*   openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout cert.pem -out cert.pem
*/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <resolv.h>

#define SIZE 1024

int main()
{   
    SSL *ssl;
    X509 *cert;
    SSL_CTX *ctx;
    const SSL_METHOD *method;
    int server_socket_descriptor, client_socket_descriptor, bytes;
    struct sockaddr_in server_addr, client_addr;
    char buffer[SIZE],reply[SIZE];
    
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();   
    
    method = SSLv3_server_method(); 
    ctx = SSL_CTX_new(method);   
    
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if ( SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    
    if ( SSL_CTX_use_PrivateKey_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate.\n");
        abort();
    }

    server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8881);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if ( bind(server_socket_descriptor, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0 )
    {
        perror("Can not bind port.");
        abort();
    }
    
    if ( listen(server_socket_descriptor, 10) != 0 )
    {
        perror("Can not configure listening port.");
        abort();
    }

    int c_addrlen = sizeof(client_addr);
    
    printf("Server Started  : %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    client_socket_descriptor = accept(server_socket_descriptor, (struct sockaddr*)&client_addr, &c_addrlen); 
    
    printf("Connection: %s:%d\n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    ssl = SSL_new(ctx);              
    SSL_set_fd(ssl, client_socket_descriptor); 
        
    if ( SSL_accept(ssl) == -1 )
    {
        ERR_print_errors_fp(stderr);
    }     
    else
    {
        cert = SSL_get_peer_certificate(ssl);
        if ( cert != NULL )
        {
            printf("Server certificates:\n\n");
            printf("Subject: \n%s\n", X509_NAME_oneline(X509_get_subject_name(cert), 0, 0));
            printf("Issuer: \n%s\n", X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0));
            X509_free(cert);
        }
        else
        {
            printf("No certificates.\n");       
        }

        while(1)
        {
            bytes = SSL_read(ssl, buffer, sizeof(buffer)); 
            if ( bytes > 0 )
            {
                buffer[bytes] = 0;
                printf("Client message:%s", buffer);
                strcpy(reply,buffer);
                SSL_write(ssl, reply, strlen(reply));
            }
            else
            {
                ERR_print_errors_fp(stderr);
            }
        }
    }
    int sd = SSL_get_fd(ssl);     
    SSL_free(ssl);       
    close(sd);

    close(server_socket_descriptor);
    SSL_CTX_free(ctx);
}
