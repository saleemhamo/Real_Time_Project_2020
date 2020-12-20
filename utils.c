#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVERPORT 2251
#define HOSTIP "localhost"
#define MEMORYCAPACITY 5597
#define RED "\x1B[31m"
#define RESET "\x1B[0m"
#define BLU "\x1B[34m"
#define CYN "\x1B[36m"

enum OPERATION
{
    CREATE,
    READ,
    WRITE,
    COPY,
    LOCK,
    UNLOCK,
    TALKTO,
    QUIT
};

enum CHANGE
{
    LOCK_CHANGE,
    UNLOCK_CHANGE,
    CONTENT_CHANGE,
    SHAREDBY_CHANGE
};

struct Change
{
    enum CHANGE type;
    int memId;
    char content[MEMORYCAPACITY];
    int updateContentIndex;
    int newMemberId;
    int lockedBy;
};

struct Request
{
    int portNumber;
    int memId;
    enum OPERATION type;
};

struct Client
{
    int portNumber;
    char *ipAdress; // "190.255.255.255"
};
struct ClientListItem
{
    struct Client client;
    struct ClientListItem *next;
};
struct Member
{
    int data;
    struct Member *next;
};
struct Memory
{
    int memoryID;
    char content[MEMORYCAPACITY];
    int lockedBy;

    struct Member *sharedBy;
    struct Memory *next;
};

///////////////////////////////////////// Socket Handling  //////////////////////////////////////
int createSocket(int PORT)
{
    /* 
this function will make a socket with intended client/server
@params - host name to be contacted  
Returns - socker file discriptor 
*/
    int sock, len;
    static struct sockaddr_in host_adr;
    char hello[10];
    char buffer[1024];
    struct hostent *host;
    host = gethostbyname(HOSTIP); //returns the ip and the address to connect to
    if (host == (struct hostent *)NULL)
    {
        perror("gethostbyname ");
        exit(2);
    }
    //configuration adress
    memset(&host_adr, 0, sizeof(host_adr));
    host_adr.sin_family = AF_INET;
    memcpy(&host_adr.sin_addr, host->h_addr, host->h_length);
    host_adr.sin_port = htons(PORT);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socker creation error");
        exit(3);
    }
    if (connect(sock, (struct sockaddr *)&host_adr, sizeof(host_adr)) < 0)
    {
        perror(RED"CONNECTION ERROR..."RESET);
        exit(4);
    }
    return sock;
}

struct Memory *memoryExists(struct Memory **head_ref, int memID)
{
    int memExists = 0;
    struct Memory *exist = *head_ref;
    if (*head_ref != NULL)
    { //(*head_ref != NULL)
        while (exist != NULL)
        {
            if (exist->memoryID == memID)
            {
                memExists = 1;
                break;
            }
            exist = exist->next;
        }
        return exist;
    }
    return NULL;
}