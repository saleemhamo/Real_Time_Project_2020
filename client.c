#include "utils.c"

int port;
int memId;

struct Memory *memoryHead = NULL;

void printMemory()
{
    /* 
    print all client memories with its attributes
    @params: - memory table refrence to be printed
    */
    struct Memory *memory = memoryHead;
    // system("clear");
    while (memory != NULL)
    {
        printf(BLU "---------------\n");
        printf("Memory ID : %d => \n", memory->memoryID);
        printf("CONTENT   : [%s]\n", memory->content);
        printf("Locked By : %d\n", memory->lockedBy);
        printf("Shared By : ");
        struct Member *mem = memory->sharedBy;
        while (mem != NULL)
        {
            printf("%d, ", mem->data);
            mem = mem->next;
        }
        memory = memory->next;
        printf("\n---------------\n" RESET);
    }

    char x = 'N';
    while (x != 'Y' && x != 'y')
    {
        printf("Do you want to continue? (Y/N)");
        scanf("%s", &x);
    } 
}
struct Memory *findMemory(int memoryId)
{
    struct Memory *memory = memoryHead;
    while (memory != NULL)
    {
        if (memory->memoryID == memoryId)
        {
            return memory;
        }
        memory = memory->next;
    }
    return NULL;
}
struct Change *getChangeDetails(char *buffer)
{
    char *token;
    struct Change *change = (struct Change *)malloc(sizeof(struct Change));
    token = strtok(buffer, ":");
    sscanf(token, "%d", &change->type);
    token = strtok(NULL, ":");
    sscanf(token, "%d", &change->memId);
    token = strtok(NULL, ":");
    strcpy(change->content, token);
    token = strtok(NULL, ":");
    sscanf(token, "%d", &change->updateContentIndex);
    token = strtok(NULL, ":");
    sscanf(token, "%d", &change->newMemberId);
    token = strtok(NULL, ":");
    sscanf(token, "%d", &change->lockedBy);
    return change;
}

void updateMemory(struct Change *change)
{
    struct Memory *memory = findMemory(change->memId);
    if (memory == NULL)
    {
        printf("\n ----------------------------------------------------\n");
        printf(RED "ERROR : Memory doesn't exist\n" RESET);
        return;
    }
    if (change->type == LOCK_CHANGE)
    {
        memory->lockedBy = change->lockedBy;
        // printf("\n ----------------------------------------------------\n");
        // printf(RED "UPDATE: Memory %d Changed... Locked By %d!\n" RESET, change->memId, change->lockedBy);
    }
    else if (change->type == UNLOCK_CHANGE)
    {
        memory->lockedBy = 0;
        // printf("\n ----------------------------------------------------\n");
        // printf(RED "UPDATE: Memory %d Changed... Unlocked!\n" RESET, change->memId);
    }
    else if (change->type == CONTENT_CHANGE)
    {
        for (int i = 0; i < strlen(change->content); i++)
        {
            memory->content[change->updateContentIndex + i] = change->content[i];
        }
        // printf("\n ----------------------------------------------------\n");
        // printf(RED "UPDATE: Memory %d Changed... Content |%s| is added by %d!\n" RESET, change->memId, change->content, memory->lockedBy);
    }
    else if (change->type == SHAREDBY_CHANGE)
    {
        struct Member *new_member = (struct Member *)malloc(sizeof(struct Member));
        new_member->data = change->newMemberId;
        new_member->next = NULL;
        struct Member *membersListPtr = memory->sharedBy;
        while (membersListPtr->next != NULL)
        {
            membersListPtr = membersListPtr->next;
        }
        membersListPtr->next = new_member;
        // printf("\n ----------------------------------------------------\n");
        // printf(RED "UPDATE: Memory %d Changed... Client %d is now sharing it!\n" RESET, change->memId, change->newMemberId);
    }
    else
    {
        printf("Not Defined Type!");
        return;
    }

    /// print updates
    // printMemory();
    printf(RED "UPDATE: Memory %d has an update!\n" RESET, change->memId);
    printf("----------------------------------------------------\n");
}

///////////////////////////////////////// Listen function ////////////////////////////////////////
void listenHandler(void *socket)
{
    int new_socket = *((int *)socket);
    char buffer[1024];
    read(new_socket, buffer, 1024);
    struct Change *change = getChangeDetails(buffer);
    if (port != change->newMemberId && port != change->lockedBy)
    {
        updateMemory(change);
    }
}

void startListinging()
{
    int orig_sock, new_sock;
    socklen_t clnt_len;
    struct sockaddr_in clnt_adr, serv_adr;
    int len, i;

    if ((orig_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Generation Socket error");
        exit(1);
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY); //basically my ip address
    serv_adr.sin_port = htons(port);              //initiating port number

    if (bind(orig_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) < 0)
    {
        perror("Bind Socket error");
        close(orig_sock);
        exit(2);
    }

    if (listen(orig_sock, 10) < 0)
    {
        perror("Listen Socket error");
        close(orig_sock);
        exit(3);
    }

    while (1)
    {
        if ((new_sock = accept(orig_sock, (struct sockaddr *)&clnt_adr, &clnt_len)) < 0)
        {
            perror("Accept Error");
            close(orig_sock);
            exit(4);
        }
        pthread_t thread;
        pthread_create(&thread, NULL, (void *)listenHandler, (void *)&new_sock);
    }
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
void signal_catcher(int the_sig)
{
    printf("\nSignal %d receivedd.\n", the_sig);
    struct Request request = initializeRequest(QUIT);
    char sendBuffer[100];
    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(SERVERPORT);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    exit(1);
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

    if (atoi(readBuffer) == -1)
        printf(RED "ERROR : Memory doesn't exist\n" RESET);
    else if (atoi(readBuffer) == -2)
        printf(RED "ERROR : Client is not member of the memory\n" RESET);
    else
    {
        struct Memory *memory = findMemory(request.memId);
        if (memory != NULL)
        {
            memory->lockedBy = atoi(readBuffer);
        }
    }
    printf("\n ---------------------------\n");
    printf(BLU "Memory %d LOCKED\n" RESET, request.memId);
    printMemory();
    printf("\n ---------------------------\n");
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

    if (atoi(readBuffer) == -1)
        printf(RED "ERROR : Memory doesn't exist\n" RESET);
    else if (atoi(readBuffer) == -2)
        printf(RED "ERROR : Client is not member of the memory\n" RESET);

    else
    {
        struct Memory *memory = findMemory(request.memId);
        if (memory != NULL)
        {
            memory->lockedBy = 0;
        }
    }
    printf("\n ---------------------------\n");
    printf(BLU "Memory %d UNLOCKED\n" RESET, request.memId);
    printMemory();
    printf("\n ---------------------------\n");
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

    if (atoi(readBuffer) == -1)
        printf(RED "ERROR : Memory doesn't exist\n" RESET);
    else if (atoi(readBuffer) == -2)
        printf(RED "ERROR : Client is not member of the memory\n" RESET);
    else if (atoi(readBuffer) == -3)
        printf(RED "ERROR : Memory is locked by another client\n" RESET);
    else
    {
        struct Memory *memory = findMemory(request.memId);
        if (memory != NULL)
        {
            strcpy(memory->content, readBuffer);
        }
    }

    printf("\n ---------------------------\n");
    printf(BLU "Memory %d COPIED\n" RESET, request.memId);
    printMemory();
    printf("\n ---------------------------\n");

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
        token = strtok(NULL, ":");
    }
}

void makeNewMemory(char *content, char *members)
{
    /*
    make a memory in the client with fields
    */
    // struct Memory **head_ref;

    struct Memory *newMemory = (struct Memory *)malloc(sizeof(struct Memory));
    newMemory->memoryID = memId;
    strcpy(newMemory->content, content);
    restructStringAsMembers(&newMemory->sharedBy, members);
    newMemory->next = (memoryHead);
    (memoryHead) = newMemory;
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
    if (atoi(readBuffer) == -1)
    {
        //unsucessful creation
        perror("error in connection");
        exit(1);
    }
    else if (atoi(readBuffer) == -4)
    {
        printf(RED "ERROR : Client is already in the memory\n" RESET);
    }
    else if (atoi(readBuffer) == 1)
    {
        //create a new memory with memid and one member with no content
        sprintf(readBuffer, "%d:", port);
        makeNewMemory("", readBuffer);
        // printMemory();
    }
    else
    {
        //already exists memroy
        makeNewMemory("", readBuffer);
        //create a new memory for this client with memid
        //request a copy that make lock then copy
        copyMemory();
    }
    printf("\n ---------------------------\n");
    printf(BLU "Memory %d CREATED\n" RESET, request.memId);
    printMemory();
    printf("\n ---------------------------\n");
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

    if (atoi(readBuffer) == -1)
        printf(RED "ERROR : Mmory doesn't exist\n" RESET);
    else if (atoi(readBuffer) == -2)
        printf(RED "ERROR : Client is not member of the memory\n" RESET);
    else if (atoi(readBuffer) == -3)
        printf(RED "ERROR : Memory is locked by another client\n" RESET);

    else
    {
        struct Memory *memory = findMemory(request.memId);
        if (memory != NULL)
        {
            strcpy(memory->content, readBuffer);
            printf("Memory %d is read... Memory Content -> |%s|", memory->memoryID, memory->content);
        }
    }
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

    if (atoi(readBuffer) == -1)
        printf(RED "ERROR : Memory doesn't exist\n" RESET);
    else if (atoi(readBuffer) == -2)
        printf(RED "ERROR : Client is not member of the memory\n" RESET);
    else
    {
        printf("------------------------------\n");
        printf("Enter data to be written:\n");
        scanf("%s", sendBuffer);
        send(sock, sendBuffer, sizeof(sendBuffer), 0);
        read(sock, readBuffer, 1024); //index of new data
    }

    printf("\n ---------------------------\n");
    printf(BLU "Memory %d UPDATED\n" RESET, request.memId);
    printMemory();
    printf("\n ---------------------------\n");

    unlockMemory();
}

void talkTo(int portToTalk)
{
    //testing purpose
    struct Request request = initializeRequest(TALKTO);
    char readBuffer[100];
    char sendBuffer[1024]; // with updates

    sprintf(sendBuffer, "%d:%d:%d", request.portNumber, request.memId, request.type);
    int sock = createSocket(portToTalk);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send client id
    read(sock, readBuffer, 1024);
    if (atoi(readBuffer) == -1)
    {
        perror("error in connection");
        exit(1);
    }
    else
    {
        printf("Connected [%s] \n", readBuffer);
    }
}

///////////////////////////////////////// Main function ////////////////////////////////////////
int main()
{
    system("clear");
    printf("---------------------------------\n");
    printf("Enter your PORT number (Clinet)\n");
    scanf("%d", &port);

    int ans;
    int portToTalk;

    pthread_t listeningThread;
    pthread_create(&listeningThread, NULL, (void *)startListinging, NULL);

    if (signal(2, signal_catcher) == SIG_ERR)
    {
        perror("Sigset can not set SIGQUIT");
        exit(1);
    }
    if (signal(3, signal_catcher) == SIG_ERR)
    {
        perror("Sigset can not set SIGQUIT");
        exit(1);
    }

    while (1)
    {
        system("clear");
        printf("Wait...\n");
        sleep(3);
        system("clear");
        printf("---------------------------------\n");
        printf("Enter memory ID you want to work on\n");
        scanf("%d", &memId);
        printf("---------------------------------\n");
        printf("Enter the type of request\n");
        printf(CYN " --------------------------------\n");
        printf("| 1- Create a Shared Memory      |\n");
        printf("| 2- Read a Shared Memory        |\n");
        printf("| 3- Write into a Shared Memory  |\n");
        printf("| 4- Copy a Shared Memory        |\n");
        printf("| 5- Lock Memory                 |\n");
        printf("| 6- Unlock Memory               |\n");
        printf("| 7- Talk To                     |\n");
        printf("| 8- Print Memories              |\n");
        printf(" --------------------------------\n" RESET);

        scanf("%d", &ans);

        system("clear");
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
        case 5: // LOCK
            lockMemory();
            break;

        case 6: // UNLOCK
            unlockMemory();
            break;
        case 7:
            printf("enter the prot you want to talk\n");
            scanf("%d", &portToTalk);
            talkTo(portToTalk);
            break;
        case 8:
            printMemory();
            break;
        default:
            printf("Faild input\n");
            break;
        }
    }

    return 0;
}