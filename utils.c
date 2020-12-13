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
};
struct Member
{
    int data;
    struct Member *next;
};
struct Memory
{
    int memNumber;
    char content [2048];
    int lockedBy;
    struct Member* sharedBy;
    struct Memory* next;
};

struct MemoryTable
{
    int memoryID;
    struct Member *sharedBy;
    struct MemoryTable *next;
};
