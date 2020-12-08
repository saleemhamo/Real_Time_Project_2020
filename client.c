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