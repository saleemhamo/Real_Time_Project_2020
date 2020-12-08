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

*/



#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9999

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
    char readBuffer[20];
    read( new_socket , readBuffer, 1024);
    
    
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
    serv_adr.sin_port        = htons(PORT);     //initiating port number

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
        clnt_len = sizeof(clnt_adr);
        if ( (new_sock = accept(orig_sock, (struct sockaddr *) &clnt_adr,&clnt_len)) < 0 ) {
            perror("accept error");
            close(orig_sock);
            exit(4);
        }
        pthread_t  thread;
        pthread_create(&thread,  NULL,  (void *) serverHandler,  (void*) &new_sock); 
    }
    
    return 0;
}