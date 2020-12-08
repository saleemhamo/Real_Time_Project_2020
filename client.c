/*

*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>




#define PORT 9999
#define SERVERIP "localhost"

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

int makeSocket(char hostAdress[]){
/* 
this function will make a socket with intended client/server
@params - host name to be contacted  
Returns - socker file discriptor 
*/
    int sock, len;
    static struct sockaddr_in host_adr;

    struct hostent *host;
    host = gethostbyname(hostAdress);    //returns the ip and the address to connect to
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
        perror("connect error");
        exit(4);
    }
    return sock;
}


int main(){

    int type;
    int ip;
    int memId;

    while(1){
        printf("Please enter your IP\n");
        scanf("%d",&ip);
        printf("Please enter memory ID you want to work on");
        scanf("%d",&memId);
        printf("Enter type of your request\n");
        printf("1- create a shared memory\n");
        printf("2- \n");
        scanf("%d",&type);

        switch (type)
        {
        case 1:
            createMemory(ip, memId);
            break;
        
        default:
            printf("Faild input");
            break;
        }
    }


    return 0 ;
}

void createMemory(int ip, int memId){



}