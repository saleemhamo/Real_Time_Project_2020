#include "utils.c"

int port;
int memId;

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

    sprintf(hello, "%d", port);

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
        perror("connect error, run the server first");
        exit(4);
    }
    return sock;
}

///////////////////////////////////////// Main Operation ////////////////////////////////////////
struct Request initializeRequest(int requestType)
{
    struct Request *request = (struct Request *)malloc(sizeof(struct Request));
    request->portNumber = port;
    request->memId = memId;
    request->type = requestType;
    return *request;
}

void createMemory()
{
    /*
    expected: 0 on sucess not existed mem
            ..... on sucess existed mem
            -1 on failure 
    */

    struct Request request = initializeRequest(CREATE);
    char readBuffer[1024];
    char sendBuffer[100];
    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);
    printf("I received [%s]\n", readBuffer);
    // if(atoi(readBuffer) == -1){
    //     perror("error in connection");
    //     exit(1);
    // }
    // else
    // {
    //     printf("connected\n");
    // }
}

void readMemory()
{
    struct Request request = initializeRequest(READ);
    char readBuffer[1024];
    char sendBuffer[100];
    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);

    //Rad Memory Logic if not locked ....
    // printf("I received [%s]\n", readBuffer);
}

void writeIntoMemory()
{
    struct Request request = initializeRequest(WRITE);
    char readBuffer[1024];
    char sendBuffer[100]; // with updates

    // add logic of updating memory

    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);
}

void copyMemory()
{
    struct Request request = initializeRequest(COPY);
    char readBuffer[1024];
    char sendBuffer[100];
    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);
}

void talkTo(int portToTalk)
{
    // //testing purpose
    // int type = 2;
    // char readBuffer[1024];
    // char sendBuffer[100];
    // sprintf(sendBuffer, "%d:%d:%d", port, memId, type);

    // int sock = createSocket(portToTalk);
    // send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    // read(sock, readBuffer, 1024);
    // if (atoi(readBuffer) == -1)
    // {
    //     perror("error in connection");
    //     exit(1);
    // }
    // else
    // {
    //     printf("connected\n");
    // }
    // printf("after make socket in create memory\n");
}

///////////////////////////////////////// Main function ////////////////////////////////////////
int main()
{
    int ans;
    printf("Enter your PORT number (Clinet)\n");
    scanf("%d", &port);

    while (1)
    {

        printf("Enter memory ID you want to work on\n");
        scanf("%d", &memId);
        printf("Enter the type of request\n");
        printf(" --------------------------------\n");
        printf("| 1- Create a Shared Memory      |\n");
        printf("| 2- Read a Shared Memory        |\n");
        printf("| 3- Write into a Shared Memory  |\n");
        printf("| 4- Copy a Shared Memory        |\n");
        printf("| 5- talk to client              |\n");
        printf(" --------------------------------\n");

        scanf("%d", &ans);

        switch (ans)
        {
        case 1: // CREATE
            printf("insdie case 1\n");
            createMemory();
            printf("after create memory in case 1\n");
            break;

        case 2: // READ
            readMemory();
            break;

        case 3: // WRITE
            writeIntoMemory();
            break;

        case 4: // COPY
            copyMemory();
            break;

        case 5:
            // int portToTalk;
            // printf("enter the prot you want to talk\n");
            // scanf("%d", &portToTalk);
            // talkTo(portToTalk);
            break;

        default:
            printf("Faild input\n");
            break;
        }
    }

    return 0;
}