#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


// #define PORT 9999
#define SERVERPORT 9999
#define HOSTIP "localhost"
#define MEMORYCAPACITY 2048
#define RED   "\x1B[31m"
#define RESET "\x1B[0m"
#define BLU   "\x1B[34m"
#define CYN   "\x1B[36m"

enum OPERATION
{
    CREATE,
    READ,
    WRITE,
    COPY,
    LOCK,
    UNLOCK
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
    char* ipAdress; // "190.255.255.255"
};
struct ClientListItem {
    struct Client client;
    struct ClientListItem* next;
};
struct Member
{
    int data;
    struct Member *next;
};
struct Memory
{
    int memoryID;
    char content [MEMORYCAPACITY];
    int lockedBy;

    struct Member* sharedBy;
    struct Memory* next;
};

// struct Memory
// {
//     int memoryID;
//     struct Member *sharedBy;
//     struct Memory *next;
// };

