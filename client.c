/*

*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>




// #define PORT 9999
// #define SERVERIP "localhost"
#define SERVERPORT 9999
#define HOSTIP "localhost"

struct Member 
{   
    int data;
    struct Member* next;
};
struct Memory
{
    int memNumber;
    char content [2048];
    int lockedBy;
    struct Node* sharedBy;
    struct Memory* next;
};
//////
/////
//////
int port;
int memId;

int makeSocket(int PORT){
/* 
this function will make a socket with intended client/server
@params - host name to be contacted  
Returns - socker file discriptor 
*/
    int sock, len;
    static struct sockaddr_in host_adr;
    char hello[10];
    char buffer[1024];

    
    sprintf(hello,"%d",port);

    struct hostent *host;
    host = gethostbyname(HOSTIP);    //returns the ip and the address to connect to
    if ( host == (struct hostent *) NULL ) {
        perror("gethostbyname ");
        exit(2);
    }
    //configuration adress
    memset(&host_adr, 0, sizeof(host_adr));
    host_adr.sin_family      = AF_INET;
    memcpy(&host_adr.sin_addr, host->h_addr, host->h_length);
    host_adr.sin_port        = htons(PORT);

    if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("socker creation error");
        exit(3);
    }
    if ( connect(sock, (struct sockaddr *) &host_adr, sizeof(host_adr)) < 0 ) {
        perror("connect error, run the server first");
        exit(4);
    }
    return sock;

    
}


void createMemory(){
    /*
    expected: 0 on sucess not existed mem
            ..... on sucess existed mem
            -1 on failure 
    */
    int type = 1;
    char readBuffer[1024];
    char sendBuffer[100];
    sprintf(sendBuffer,"%d:%d:%d",port,memId,type);

    int sock = makeSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer),0);//send client id
    read(sock , readBuffer, 1024);
    if(atoi(readBuffer) == -1){
        perror("error in connection");
        exit(1);
    }
    else
    {
        printf("connected\n");
    }
    // printf("after make socket in create memory\n");
}

void talkTo(int portToTalk){
//testing purpose
    int type = 2;
    char readBuffer[1024];
    char sendBuffer[100];
    sprintf(sendBuffer,"%d:%d:%d",port,memId,type);

    int sock = makeSocket(portToTalk);
    send(sock, sendBuffer, sizeof(sendBuffer),0);//send client id
    read(sock , readBuffer, 1024);
    if(atoi(readBuffer) == -1){
        perror("error in connection");
        exit(1);
    }
    else
    {
        printf("connected\n");
    }
    // printf("after make socket in create memory\n");
}



int main(){
    int ans;
    printf("Please enter your PORT number\n");
    scanf("%d",&port);
    

    while(1){
        
        printf("Please enter memory ID you want to work on\n");
        scanf("%d",&memId);
        printf("Enter type of your request\n");
        printf("1- create a shared memory\n");
        printf("2- talk to client \n");
        scanf("%d",&ans);

        switch (ans)
        {
        case 1:
            printf("insdie case 1\n");
            createMemory();
            printf("after create memory in case 1\n");
            break;
        
        case 2:
            printf("insdie case 1\n");
            int portToTalk;
            printf("enter the prot you want to talk\n");
            scanf("%d",&portToTalk);
            talkTo(portToTalk);
            printf("after create memory in case 1\n");
            break;
        
        default:
            printf("Faild input\n");
            break;
        }
    }

    
    return 0 ;
}

