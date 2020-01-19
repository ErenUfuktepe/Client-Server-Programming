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
#include "server.h"

#define LENGTH_NAME 31
#define LENGTH_MSG 101
#define LENGTH_SEND 201

int server_sockfd = 0; 
int client_sockfd = 0;
ClientNode *root, *tail;

void close_socketfd(int sig) {
    ClientNode *tmp;
    while (root != NULL) {
        printf("\nServer is closed\n");
        close(root->data);
        tmp = root;
        root = root->link;
        free(tmp);
    }
    exit(EXIT_SUCCESS);
}

void public_group_chat_messsage(ClientNode *np, char tmp_buffer[]) {
    ClientNode *tmp = root->link;
    while (tmp != NULL) {
	if (np->data != tmp->data && strcmp(np->password, tmp->password) == 0) {
	    printf("Send to sockfd %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
        }
        tmp = tmp->link;
    }
}

void public_group_chat_cliet_handler(void *p_client) {
    int leave_flag = 0;
    char nickname[LENGTH_NAME] = {};
    char password[LENGTH_NAME] = {};
    char receive_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = {};
    ClientNode *np = (ClientNode *)p_client;
    
    if (recv(np->data, nickname, LENGTH_NAME, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("%s didn't input name.\n", np->ip);
        leave_flag = 1;
    } 
    else {
	recv(np->data, password, LENGTH_NAME, 0);
	strncpy(np->password, password, LENGTH_NAME);        
	strncpy(np->username, nickname, LENGTH_NAME);
	printf("%s(%s)(%d) join the chatroom.\n", np->username, np->ip, np->data);
        sprintf(send_buffer, "%s(%s) join the chatroom.", np->username, np->ip);
        public_group_chat_messsage(np, send_buffer);
    }

    while (1) {
        if (leave_flag) {
            break;
        }
        int receive = recv(np->data, receive_buffer, LENGTH_MSG, 0);
        if (receive > 0) {
            if (strlen(receive_buffer) == 0) {
                continue;
            }
           sprintf(send_buffer, "%s：%s from %s", np->username, receive_buffer, np->ip);
        } else if (receive == 0 || strcmp(receive_buffer, "exit") == 0) {
            printf("%s(%s)(%d) leave the chatroom.\n", np->username, np->ip, np->data);
            sprintf(send_buffer, "%s(%s) leave the chatroom.", np->username, np->ip);
            leave_flag = 1;
        } else {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        public_group_chat_messsage(np, send_buffer);
    }
 
    close(np->data);
    if (np == tail) {
        tail = np->previous;
        tail->link = NULL;
    } else {
        np->previous->link = np->link;
        np->link->previous = np->previous;
    }
    free(np);
}

int main()
{
    signal(SIGINT, close_socketfd);

    server_sockfd = socket(AF_INET , SOCK_STREAM , 0);
    
    if (server_sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_info, client_info;
    
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(8888);

    bind(server_sockfd, (struct sockaddr *)&server_info, s_addrlen);
    listen(server_sockfd, 5);

    getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("\n-------------------------------------------------------\n");
    printf("Server Started	: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("-------------------------------------------------------\n\n");
    
    root = newNode(server_sockfd, inet_ntoa(server_info.sin_addr));
    tail = root;

    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        
	getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        printf("Client %s:%d connected.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

        ClientNode *client = newNode(client_sockfd, inet_ntoa(client_info.sin_addr));
        client->previous = tail;
        tail->link = client;
        tail = client;

        pthread_t id;
        if (pthread_create(&id, NULL, (void *)public_group_chat_cliet_handler, (void *)client) != 0) {
            perror("Error occurred while creating thread.Create pthread error!\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
