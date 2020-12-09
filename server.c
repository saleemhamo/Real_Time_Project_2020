/* SERVER workflow
any client sends a request to server, the server will handle it, create a new mem if not exist, if it does then retutn all machines that share this mem\

Notes:
    + we will make two linked lists: 
        one for memory table (so we can make any number of memories), 
        another for members (so we can make any number of machines inside each table)


TRADE-OFF questions:
Does the server has a copy of memory
Does the server send a lock request to all other  machines

Assuptions:
    the client sends the memory # want to make
    any message will be sent as "ip:memId:request type"

*/



#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h> 

// #define PORT 9999
#define SERVERPORT 9999
#define HOSTIP "localhost"

struct Member 
{   
    int data;
    struct Member* next;
};
struct MemTable
{
    int memNumber;
    int lockedBy;
    struct Node* sharedBy;
    struct MemTable* next;
};


void serverHandler(void* data) {
    int new_socket = *((int*)data) ;
    char buffer[1024];
    read( new_socket , buffer, 1024);
    printf("I read buffer [%s]\n",buffer);
    //split buffer for ip, memid, type
    char* token;
    int port,memId,type;
    token = strtok (buffer, ":");
    sscanf (token, "%d", &port);
    token = strtok (NULL, ":");
    sscanf (token, "%d", &memId);
    token = strtok (NULL, ":");
    sscanf (token, "%d", &type);
    printf("I reeived a message type %d of memId %d from %d\n",type,memId,port);
    send(new_socket, "1", sizeof("1"), 0);


    

    
}


main(void){

    int orig_sock, new_sock, clnt_len;
    struct sockaddr_in clnt_adr, serv_adr;
    int len, i;

    if ( (orig_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Generation Socket error");
        exit(1);
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family      = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY); //basically my ip address
    serv_adr.sin_port        = htons(SERVERPORT);     //initiating port number

    if ( bind(orig_sock, (struct sockaddr *) &serv_adr,sizeof(serv_adr)) < 0 ) {
        perror("bind Socket error");
        close(orig_sock);
        exit(2);
    }

    if ( listen(orig_sock, 5) < 0 ) {     //can listen for 5
        perror("listen Socket error");
        close(orig_sock);
        exit(3);
    }

    while(1){
        printf("start while loop\n");
        clnt_len = sizeof(clnt_adr);
        if ( (new_sock = accept(orig_sock, (struct sockaddr *) &clnt_adr,&clnt_len)) < 0 ) {
            perror("accept error");
            close(orig_sock);
            exit(4);
        }
        // printf("after accept\n");
        pthread_t  thread;
        // printf("before thread\n");
        pthread_create(&thread,  NULL,  (void *) serverHandler,  (void*) &new_sock); 
        pthread_join(thread, NULL);
        printf("after thread\n");
    }
    
    return 0;
}