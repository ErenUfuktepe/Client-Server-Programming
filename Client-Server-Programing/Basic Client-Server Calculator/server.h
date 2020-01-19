#ifndef SERVER
#define SERVER

typedef struct ClientNode {
    int data;
    char ip[16];
    struct ClientNode* previous;
    struct ClientNode* link;
}ClientNode;

ClientNode *newNode(int sockfd, char* ip) {
    ClientNode *np = (ClientNode *)malloc( sizeof(ClientNode) );
    np->data = sockfd;
    np->previous = NULL;
    np->link = NULL;
    strncpy(np->ip, ip, 16);
    return np;
}

#endif
