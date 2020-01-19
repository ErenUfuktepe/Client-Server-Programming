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

#define LENGTH_MSG 101
#define LENGTH_SEND 201

int server_sockfd = 0; 
int client_sockfd = 0;
ClientNode *root, *tail;

int calculate_sum(char message[LENGTH_MSG]){
    char *token[80];
    int i = 0 , j = 0;
    double result = 0;
    token[0] =  strtok(message, " ");
    
    while (token[i] != NULL) {
	i++;
	token[i] = strtok(NULL, " ");  
    }
    
    if(atoi(token[0]) == 0){  
       if(strstr(token[0] ,"+") != NULL){
    	 for (j=1; j<=i-1; j++) {
	    result = result + atoi(token[j]);
         }
       }
       else if(strstr(token[0] ,"-") != NULL){
    	 for (j=1; j<=i-1; j++) {
            if(j == 1){
		result = atoi(token[j]);
            }else{
	    	result = result -atoi(token[j]);
            }
	 }
       }
       else if(strstr(token[0] ,"x") != NULL){
    	 result = 1;
	 for (j=1; j<=i-1; j++) {
	    result = result*atoi(token[j]);
         }
       }
       else if(strstr(token[0] ,"/") != NULL){
	 result = 1;	
    	 for (j=1; j<=i-1; j++) {
	    if(j == 1){
		result = atoi(token[j]);
            }else{
	    	result = result / atoi(token[j]);
            }
         }
       }    
    }    
    else{
    	for (j=0; j<=i-1; j++) {
	   result = result + atoi(token[j]);
        }
    }
    return result;
}


void close_server(int sig) {
    ClientNode *tmp;
    while (root != NULL) {
        printf("\nServer is closed.\n");
        close(root->data);
        tmp = root;
        root = root->link;
        free(tmp);
    }
    exit(EXIT_SUCCESS);
}

void send_message (ClientNode *np, char tmp_buffer[]) {
    ClientNode *tmp = root->link;
    while (tmp != NULL) {
        if (np->data == tmp->data) {
            printf("Send to sockfd %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
        }
        tmp = tmp->link;
    }
}

void cliet_handler(void *p_client) {
    int leave_flag = 0;
    char receive_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = "Welcome to our server...";
    ClientNode *np = (ClientNode *)p_client;

    send_message(np, send_buffer);

    while (1) {
        if (leave_flag) {
            break;
        }

        int receive = recv(np->data, receive_buffer, LENGTH_MSG, 0);
        
	if (receive > 0) {
            if (strlen(receive_buffer) == 0) {
                continue;
            }
            sprintf(send_buffer, "Result：%d ",calculate_sum(receive_buffer));
        } else if (receive == 0 || strcmp(receive_buffer, "exit") == 0) {
            printf("(%s)(%d) leave the server.\n", np->ip, np->data);
            leave_flag = 1;
        } else {
            printf("Fatal Error...\n");
            leave_flag = 1;
        }
        send_message(np, send_buffer);
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
    signal(SIGINT, close_server);

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
        if (pthread_create(&id, NULL, (void *)cliet_handler, (void *)client) != 0) {
            perror("Error occurred while creating thread.\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
