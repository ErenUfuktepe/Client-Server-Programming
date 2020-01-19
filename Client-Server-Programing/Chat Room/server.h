#ifndef CLIENT
#define CLIENT

typedef struct ClientNode {
    int data;
    char password[31];
    char ip[16];
    char username[31];
    struct ClientNode* previous;
    struct ClientNode* link;
}ClientNode;

ClientNode *newNode(int sockfd, char* ip) {
    ClientNode *np = (ClientNode *)malloc( sizeof(ClientNode) );
    np->data = sockfd;
    strncpy(np->password, "NULL", 5);
    np->previous = NULL;
    np->link = NULL;
    strncpy(np->ip, ip, 16);
    strncpy(np->username, "NULL", 5);
    return np;
}

#endif 
