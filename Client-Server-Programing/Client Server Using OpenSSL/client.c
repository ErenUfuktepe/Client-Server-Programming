#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <resolv.h>
#include <netdb.h>
 
#define SIZE 1024
 
int main()
{   
    SSL *ssl;
    SSL_CTX *ctx;
    X509 *cert;
    const SSL_METHOD *method;
    char buffer[SIZE],message[SIZE], *check;
    int socket_descriptor,bytes;
    struct sockaddr_in addr;
 
    SSL_library_init();
    OpenSSL_add_all_algorithms();  
    SSL_load_error_strings();   
    
    method = SSLv3_client_method(); 
    ctx = SSL_CTX_new(method);  
    
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8881);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(socket_descriptor, (struct sockaddr*)&addr, sizeof(addr)) != 0)
    {
        close(socket_descriptor);
        perror("Connection error.");
        abort();
    }

    ssl = SSL_new(ctx);     
    SSL_set_fd(ssl, socket_descriptor);
    
    if (SSL_connect(ssl) == -1)
    {
        ERR_print_errors_fp(stderr);
    }   
    else
    {   
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
   
        cert = SSL_get_peer_certificate(ssl); 
        
        if ( cert != NULL )
        {
            printf("--------------------------------------------------------------------------\n");
            printf("Server certificates:\n\n");
            printf("Subject: \n %s \n", X509_NAME_oneline(X509_get_subject_name(cert), 0, 0));
            printf("Issuer: \n %s \n\n", X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0));
            printf("--------------------------------------------------------------------------\n");
            X509_free(cert);
        }
        else
        {
            printf("No certificates.\n");
        }
        
        while(1)
        {
            printf("Client message:");
            fgets(message,sizeof(message),stdin);
            
            SSL_write(ssl, message, strlen(message));   
            
            check = strtok(message, "\n");
            if(strcmp("QUIT",check) == 0){
                break;
            }

            bytes = SSL_read(ssl, buffer, sizeof(buffer)-1); 
            buffer[bytes] = 0;
            printf("Server message:%s", buffer);
        }
        SSL_free(ssl);    
    }

    close(socket_descriptor);       
    SSL_CTX_free(ctx);     

    return 0;
}
