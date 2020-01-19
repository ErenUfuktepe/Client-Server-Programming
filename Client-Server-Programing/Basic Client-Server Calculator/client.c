#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH_MSG 101
#define LENGTH_SEND 201

volatile sig_atomic_t flag = 0;
int sockfd = 0;

void exit_signal(int sig) {
    flag = 1;
}

void recv_result() {
    char receiveMessage[LENGTH_SEND] = {};
    while (1) {
        int receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
        if (receive > 0) {
            printf("\r%s\n", receiveMessage);
            printf("\r%s", "> ");
            fflush(stdout);
        } else if (receive == 0) {
            break;
        } else { 
            exit(EXIT_FAILURE);
        }
    }
}

void send_numbers() {
    char message[LENGTH_MSG] = {};
    while (1) {
        printf("\r%s", "> ");
    	fflush(stdout);
        while (fgets(message, LENGTH_MSG, stdin) != NULL) {
            if (strlen(message) == 0) {                
	       printf("\r%s", "> ");
               fflush(stdout);
            }
	    else {
               break;
            }
        }
        send(sockfd, message, LENGTH_MSG, 0);
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }
    exit_signal(2);
}

int main()
{
    signal(SIGINT, exit_signal);

    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_info, client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.sin_port = htons(8888);

    if (connect(sockfd, (struct sockaddr *)&server_info, s_addrlen) == -1) {
        printf("Connection to Server error!\n");
        exit(EXIT_FAILURE);
    }

    getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
    getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);

    printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

    pthread_t send_msg_thread;
    pthread_t recv_msg_thread;

    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_result, NULL) != 0 
		|| pthread_create(&send_msg_thread, NULL, (void *) send_numbers, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if(flag) {
            printf("\nClosed.\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
