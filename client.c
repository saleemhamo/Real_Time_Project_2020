#include "utils.c"

int port;
int memId;
int searchID = 0;

struct Memory *memoryHead = NULL;

void printMemories()
{
    struct Memory *memoryPtr = memoryHead;
    printf("----------------------");
    printf("Memories:");
    while (memoryPtr != NULL)
    {
        printf("Memory ID: %d\n", memoryPtr->memoryID);
        memoryPtr = memoryPtr->next;
    }
    printf("----------------------");
}

struct Memory *printMemory()
{
    /* 
    print all client memories with its attributes
    @params: - memory table refrence to be printed
    */

    // printf("start printing mem\n");

    struct Memory *memory = memoryHead;
    printf(" \n*********************************** \n");
    while (memory != NULL)
    {
        if (searchID != 0 && memory->memoryID == searchID)
        {
            return memory;
        }
        else
        {
            printf(BLU " memory id: %d => \n", memory->memoryID);
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
            printf(RESET "\n-------------------\n");
        }
    }
    // printf("end printing mem\n");
    return NULL;
}

struct Memory *findMemory(int memoryId)
{
    printf("start find\n");
    int i = 0;
    printf("Searching for: %d\n", memoryId);
    struct Memory *memory = memoryHead;
    // printf("inside find memmemememem id = %d\n",memory->memoryID);
    // printf("inside find memmemememem locked id = %d\n",memory->lockedBy);
    while (memory != NULL)
    {
        printf("%d\n\n", i);
        i++;
        if (memory->memoryID == memoryId)
        {
            printf("LKETHAAAAAAAAA\n");
            return memory;
        }
        memory = memory->next;
    }
    printf("malkeeeeethaaaaash NULLLLLLL\n");
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
    // sscanf(token, "%s", &change->content);
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

    // printf("update started********\n");
    // printf("in Update: %d:%d:%s:%d:%d:%d", change->type, change->memId, change->content, change->updateContentIndex, change->newMemberId, change->lockedBy);
    // printf("will use find inside update to find %d\n", change->memId);
    // printf("print memoey befor find()\n");
    printMemory();
    struct Memory *memory = findMemory(change->memId);
    // struct Memory *memory = memoryExists(&memoryHead, change->memId);
    // searchID= change->memId;
    // struct Memory *memory =  printMemory();
    // searchID = 0;
    // printf("Found Memory:", memory->memoryID);
    // printf("Found Memory:");
    if (memory == NULL)
    {
        printf(RED "ERROR : Memory doesn't exist\n" RESET);
        return;
    }

    if (change->type == LOCK_CHANGE)
    {
        memory->lockedBy = change->lockedBy;
        printf("entered change type\n");
    }
    else if (change->type == UNLOCK_CHANGE)
    {
        memory->lockedBy = 0;
    }
    else if (change->type == CONTENT_CHANGE)
    {
        for (int i = 0; i < strlen(change->content); i++)
        {
            memory->content[change->updateContentIndex + i] = change->content[i];
        }
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
    }
    else
    {
        printf("not defined type");
    }
    // printf("update ended ********\n");
}

///////////////////////////////////////// Listen function ////////////////////////////////////////
void listenHandler(void *socket)
{
    int new_socket = *((int *)socket);
    char buffer[1024];
    read(new_socket, buffer, 1024);
    // printf("I read buffer [%s]\n", buffer);
    struct Change *change = getChangeDetails(buffer);
    // send(new_socket, "HIII", sizeof("HIII"), 0);
    // printf("print memroy befor update in listner\n");
    printMemory();
    if (port != change->newMemberId && port != change->lockedBy)
    {
        updateMemory(change);
    }
    // printf("suppose %d\n", change->newMemberId);
    printMemory();
}

void startListinging()
{
    // printf("prot from start listen %d\n", port);
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
        perror("bind Socket error");
        close(orig_sock);
        exit(2);
    }

    if (listen(orig_sock, 10) < 0)
    { //can listen for 5
        perror("listen Socket error");
        close(orig_sock);
        exit(3);
    }

    while (1)
    {
        // printf("start while loop\n");
        // clnt_len = sizeof(clnt_adr);
        // printf("Im listening now.....\n");
        // printf("print memroy befor accept\n");
        printMemory();
        if ((new_sock = accept(orig_sock, (struct sockaddr *)&clnt_adr, &clnt_len)) < 0)
        {
            perror("accept error");
            close(orig_sock);
            exit(4);
        }
        pthread_t thread;
        // printf("start thread\n");
        pthread_create(&thread, NULL, (void *)listenHandler, (void *)&new_sock);
        // pthread_join(thread, NULL);
        // printf("after thread\n");
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
    // printf("after read in lock [%s]\n", readBuffer);

    if (atoi(readBuffer) == -1)
        printf(RED "ERROR : the memory doesn't exist\n" RESET);
    else if (atoi(readBuffer) == -2)
        printf(RED "ERROR : the client is not member of the memory\n" RESET);
    else
    {
        printf("i will use find form lock\n");
        struct Memory *memory = findMemory(request.memId);
        if (memory != NULL)
        {
            memory->lockedBy = atoi(readBuffer);
        }
    }

    printMemory();
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
        printf(RED "ERROR : the memory doesn't exist\n" RESET);
    else if (atoi(readBuffer) == -2)
        printf(RED "ERROR : the client is not member of the memory\n" RESET);

    else
    {
        printf("i will use find form unlock\n");
        struct Memory *memory = findMemory(request.memId);
        if (memory != NULL)
        {
            memory->lockedBy = 0;
        }
    }

    printMemory();
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
        printf(RED "ERROR : the memory doesn't exist\n" RESET);
    else if (atoi(readBuffer) == -2)
        printf(RED "ERROR : the client is not member of the memory\n" RESET);
    else if (atoi(readBuffer) == -3)
        printf(RED "ERROR : Memory is locked by another client\n" RESET);
    else
    {
        // printf("the content received :[%s]\n", readBuffer);
        // printf("i will use find form copy\n");
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
    printf("%s", token);
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
    // printf("I received [%s]\n", readBuffer);

    if (atoi(readBuffer) == -1)
    { //unsucessful creation
        perror("error in connection");
        exit(1);
    }
    else if (atoi(readBuffer) == -4)
    {
        printf(RED "ERROR : the client already in the memory\n" RESET);
    }
    else if (atoi(readBuffer) == 1)
    { //new memroy creation
        printf("new memory\n");
        //create a new memory with memid and one member with no content
        sprintf(readBuffer, "%d:", port);
        makeNewMemory("", readBuffer);
        printMemory();
    }
    else
    { //already exists memroy
        // lockMemory();
        makeNewMemory("", readBuffer);
        // unlockMemory();
        //create a new memory for this client with memid
        //request a copy that make lock then copy
        copyMemory();
        printMemory();
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

    if (atoi(readBuffer) == -1)
        printf(RED "ERROR : the memory doesn't exist\n" RESET);
    else if (atoi(readBuffer) == -2)
        printf(RED "ERROR : the client is not member of the memory\n" RESET);
    else if (atoi(readBuffer) == -3)
        printf(RED "ERROR : Memory is locked by another client\n" RESET);

    else
    {
        // printf("the content received :[%s]\n", readBuffer);
        // printf("i will use find form read\n");
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

    if (atoi(readBuffer) == -1)
        printf(RED "ERROR : the memory doesn't exist\n" RESET);
    else if (atoi(readBuffer) == -2)
        printf(RED "ERROR : the client is not member of the memory\n" RESET);
    else
    {
        printf("*********************\n");
        printf("Enter data to be written:\n");
        scanf("%s", sendBuffer);
        send(sock, sendBuffer, sizeof(sendBuffer), 0);
        read(sock, readBuffer, 1024); //index of new data
        printf("index of new written data [%s]*******\n", readBuffer);
    }
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
        printf("connected [%s] \n", readBuffer);
    }
    printf("after make socket in create memory\n");
}

///////////////////////////////////////// Main function ////////////////////////////////////////
int main()
{

    // memoryHead = mmap(NULL, sizeof *memoryHead, PROT_READ | PROT_WRITE,
    //                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    printf("Enter your PORT number (Clinet)\n");
    scanf("%d", &port);
    // pid_t pid = fork(); // create child process to listen for conecctions.
    int ans;
    int portToTalk;
    // int pid = 1;

    pthread_t listeningThread;
    pthread_create(&listeningThread, NULL, (void *)startListinging, NULL);
    // pthread_create(&listeningThread, NULL, (void *)startListinging, (void *)&port);
    // pthread_join(listeningThread, NULL);

    while (1)
    {
        // system("clear");
        printf("Enter memory ID you want to work on\n");
        scanf("%d", &memId);
        printf("Enter the type of request\n");
        printf(CYN " --------------------------------\n");
        printf("| 1- Create a Shared Memory      |\n");
        printf("| 2- Read a Shared Memory        |\n");
        printf("| 3- Write into a Shared Memory  |\n");
        printf("| 4- Copy a Shared Memory        |\n");
        printf("| 5- lock memory                 |\n");
        printf("| 6- unlock memory               |\n");
        printf("| 7- Talk to                     |\n");
        printf("| 8- Print Memories              |\n");
        printf(" --------------------------------\n" RESET);

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