#include "utils.c"

int port;
int memId;

struct Memory *memoryHead = NULL;

struct Memory *findMemory(int memoryId)
{

    struct Memory *memoryPtr = memoryHead;
    while (memoryPtr != NULL)
    {
        if (memoryPtr->memoryID == memoryId)
        {
            return memoryPtr;
        }
    }
    return NULL;
}

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
    // sprintf(hello, "%d", port);
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

void printMemory(struct Memory *memory)
{
    /* 
    print all client memories with its attributes
    @params: - memory table refrence to be printed
    */

    printf(" \n*********************************** \n");
    while (memory != NULL)
    {
        
        printf(BLU" memory id: %d => \n", memory->memoryID);
        printf("CONTENT: [%s]\n", memory->content);
        printf("locked by: %d\n", memory->lockedBy);
        printf("shared by: ");
        struct Member *mem = memory->sharedBy;
        while (mem != NULL)
        {
            printf(" %d ,", mem->data);
            mem = mem->next;
        }
        // printf("\n \n");
        memory = memory->next;
        printf(RESET"\n-------------------\n");
    }
}

struct Request initializeRequest(int requestType)
{
    struct Request *request = (struct Request *)malloc(sizeof(struct Request));
    request->portNumber = port;
    request->memId = memId;
    request->type = requestType;
    return *request;
}

void lockMemory()
{
    /*
    server
    return -1 when the memory doesn't exist
            -2 when the client is not member of the memry
            client prot number when sucess
    */

    struct Request request = initializeRequest(LOCK);
    char readBuffer[1024];
    char sendBuffer[100];
    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);
    printf("after read in lock [%s]\n",readBuffer);

    if(atoi(readBuffer) == -1)
        printf(RED "ERROR : the memory doesn't exist\n" RESET);
    else if(atoi(readBuffer) == -2)
        printf(RED "ERROR : the client is not member of the memory\n" RESET);
    else
    {
        struct Memory *memory = findMemory(request.memId);
        if (memory != NULL)
        {
            memory->lockedBy = atoi(readBuffer);
        }
    }

    printMemory(memoryHead);
}

void unlockMemory()
{
    /*
    server
    return -1 when the memory doesn't exist
            -2 when the client is not member of the memry
            0 when sucess
    */
    struct Request request = initializeRequest(UNLOCK);
    char readBuffer[1024];
    char sendBuffer[100];
    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);

    if(atoi(readBuffer) == -1)
        printf(RED "ERROR : the memory doesn't exist\n" RESET);
    else if(atoi(readBuffer) == -2)
        printf(RED "ERROR : the client is not member of the memory\n" RESET);

    else
    {
        struct Memory *memory = findMemory(request.memId);
        if (memory != NULL)
        {
            memory->lockedBy = 0;
        }
    }


    printMemory(memoryHead);
}

void copyMemory()
{
    /*
    server return -1 when the memory doesn't exist
                  -2 when the client is not member of the memry
                  -3 when the memory is locked by another
                  [content] when sucess
    */
    lockMemory();
    struct Request request = initializeRequest(COPY);
    char readBuffer[1024];
    char sendBuffer[100];
    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);

    if(atoi(readBuffer) == -1)
        printf(RED "ERROR : the memory doesn't exist\n" RESET);
    else if(atoi(readBuffer) == -2)
        printf(RED "ERROR : the client is not member of the memory\n" RESET);
    else if(atoi(readBuffer) == -3)
        printf(RED "ERROR : Memory is locked by another client\n" RESET);
    else{
        printf("the content received :[%s]\n",readBuffer);
        struct Memory *memory = findMemory(request.memId);
        if (memory != NULL)
        {
            strcpy(memory->content, readBuffer);
        }
    }
    unlockMemory();
}

void restructStringAsMembers(struct Member **head_ref, char *members)
{
    /*
    transform the members string to struct members, add them to memory
    @params: - head_ref: member refrence of new memory (where to add the members)
             - members: members of memory as string "client1:client2:..."
    */
    char *token;
    int num;
    token = strtok(members, ":");
    num = atoi(token);
    while (token != NULL)
    {
        num = atoi(token);
        struct Member *newMember = (struct Member *)malloc(sizeof(struct Member));
        newMember->data = num;
        newMember->next = (*head_ref);
        (*head_ref) = newMember;
        printf("[%d]", num);
        token = strtok(NULL, ":");
    }
}

void makeNewMemory(struct Memory **head_ref, char *content, char *members)
{
    /*
    make a memory in the client with fields
    */
    struct Memory *newMemory = (struct Memory *)malloc(sizeof(struct Memory));
    newMemory->memoryID = memId;
    strcpy(newMemory->content, content);
    restructStringAsMembers(&newMemory->sharedBy, members);
    newMemory->next = (*head_ref);
    (*head_ref) = newMemory;
}

void createMemoryRequest()
{
    /*
    expected: 1 on sucess not existed mem
             -1 on failure 
             -4 if the client already in the memroy
             "client1:client2:...." on sucess existed mem
    */
    struct Request request = initializeRequest(CREATE);
    char readBuffer[1024];
    char sendBuffer[100];
    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);

    //handel if the client already has the memory ...........................

    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);
    printf("I received [%s]\n", readBuffer);

    if (atoi(readBuffer) == -1)
    { //unsucessful creation
        perror("error in connection");
        exit(1);
    }
    else if (atoi(readBuffer) == -4){
        printf(RED "ERROR : the client already in the memory\n" RESET);
    }
    else if (atoi(readBuffer) == 1)
    { //new memroy creation
        printf("new memory\n");
        //create a new memory with memid and one member with no content
        sprintf(readBuffer, "%d:", port);
        makeNewMemory(&memoryHead, "", readBuffer);
        printMemory(memoryHead);
    }
    else
    { //already exists memroy
        makeNewMemory(&memoryHead, "dummy content", readBuffer);
        //create a new memory for this client with memid
        //request a copy that make lock then copy
        copyMemory();
        printMemory(memoryHead);
    }
}

void readMemory()
{
    /*
    server return -1 when the memory doesn't exist
                  -2 when the client is not member of the memry
                  -3 when the memory is locked by another
                  [content] when sucess
    */
    struct Request request = initializeRequest(READ);
    char readBuffer[1024];
    char sendBuffer[100];
    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);

    if(atoi(readBuffer) == -1)
        printf(RED "ERROR : the memory doesn't exist\n" RESET);
    else if(atoi(readBuffer) == -2)
        printf(RED "ERROR : the client is not member of the memory\n" RESET);
    else if(atoi(readBuffer) == -3)
        printf(RED "ERROR : Memory is locked by another client\n" RESET);

    else{
        printf("the content received :[%s]\n",readBuffer);
        struct Memory *memory = findMemory(request.memId);
        if (memory != NULL)
        {
            strcpy(memory->content, readBuffer);
        }
    }

    //Rad Memory Logic if not locked ....
    // printf("I received [%s]\n", readBuffer);
}

void writeIntoMemory()
{
    lockMemory();
    struct Request request = initializeRequest(WRITE);
    char readBuffer[100];
    char sendBuffer[1024]; // with updates

    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);

    if(atoi(readBuffer) == -1)
        printf(RED "ERROR : the memory doesn't exist\n" RESET);
    else if(atoi(readBuffer) == -2)
        printf(RED "ERROR : the client is not member of the memory\n" RESET);
    else{
        printf("*********************\n");
        printf("Enter data to be written:\n");
        scanf("%s",sendBuffer);
        send(sock, sendBuffer, sizeof(sendBuffer), 0);
        read(sock, readBuffer, 1024);//index of new data
        printf("******[%s]*******\n",readBuffer); 
    }
    unlockMemory();
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
        printf(CYN" --------------------------------\n");
        printf("| 1- Create a Shared Memory      |\n");
        printf("| 2- Read a Shared Memory        |\n");
        printf("| 3- Write into a Shared Memory  |\n");
        printf("| 4- Copy a Shared Memory        |\n");
        printf("| 5- lock memory                 |\n");
        printf("| 6- unlock memory               |\n");
        printf(" --------------------------------\n"RESET);

        scanf("%d", &ans);

        switch (ans)
        {
        case 1: // CREATE
            createMemoryRequest();
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

        case 5: // COPY
            lockMemory();
            break;

        case 6: // COPY
            unlockMemory();
            break;

        case 7:
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