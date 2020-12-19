/* SERVER workflow
any client sends a request to server, the server will handle it, create a new mem if not exist, if it does then return all machines that share this mem\

Notes:
    + we will make two linked lists: 
        one for memory table (so we can make any number of memories), 
        another for members (so we can make any number of machines inside each table)


TRADE-OFF questions:
Does the server has a copy of memory
Does the server send a lock request to all other  machines

Assuptions:
    the client sends the memory # want to make
    any message will be sent as "port:memId:request type"

*/

//must handel if the client didn't create the memory
//inform client about changes in memroy
//now the client can lock,unlock,read,copy the memory even if not a member in it Done
// the client can be added to same memory many times    Done
//print error in RED    Done

#include "utils.c"

///////////////////////////////////////// MemoryTable ////////////////////////////////////////

struct Memory *tableHead = NULL;

struct Change *createChangeObject(enum CHANGE type, int memId, char content[], int updateContentIndex, int newMemberId, int lockedBy)
{
    struct Change *change = (struct Change *)malloc(sizeof(struct Change));
    change->type = type;
    change->memId = memId;
    change->updateContentIndex = updateContentIndex;
    change->newMemberId = newMemberId;
    change->lockedBy = lockedBy;
    strcpy(change->content, content);
    // for (int i = 0; i < strlen(content); i++)
    // {
    //     change->content[i] = content[i];
    // }
    // change->content[strlen(change->content)] = '\0';
    return change;
}

void notifyUpdates(struct Change *change, int clientPort)
{
    char sendBuffer[1024];
    sprintf(sendBuffer, "%d:%d:%s:%d:%d:%d", change->type, change->memId, change->content, change->updateContentIndex, change->newMemberId, change->lockedBy);
    // sprintf(sendBuffer, "%d:%d:%d:%d:%d", change->type, change->memId,change->updateContentIndex, change->newMemberId, change->lockedBy);

    int sock = createSocket(clientPort);
    printf("Inside notify: Client: %d, Memory: %d\n", clientPort, change->memId);
    send(sock, sendBuffer, sizeof(sendBuffer), 0); //send updates
}

void addNewMember(struct Member **head_ref, int new_data)
{
    struct Member *new_node = (struct Member *)malloc(sizeof(struct Member));
    new_node->data = new_data;
    new_node->next = (*head_ref);
    (*head_ref) = new_node;
}

void sendMembersToClientAsString(struct Member **head_ref, int socket, int memID)
{
    /*
    sends all clients that shared this memory as : "client1:client2:...."
    */
    char buffer[2000];
    char port[40];
    struct Member *members = *head_ref;
    while (members != NULL)
    {
        sprintf(port, "%d:", members->data);
        strcat(buffer, port);
        members = members->next;
    }

    send(socket, buffer, sizeof(buffer), 1024);
}

void printTable(struct Memory *table)
{
    /* 
    print all tables and its content
    */
    printf("start print \n*************************** \n");
    while (table != NULL)
    {
        printf(BLU "| %d | => ", table->memoryID);
        printf("locked by: %d\n", table->lockedBy);
        printf("content [%s]\n", table->content);
        printf("Members: ");
        struct Member *mem = table->sharedBy;
        while (mem != NULL)
        {
            printf(" %d ,", mem->data);
            mem = mem->next;
        }
        printf(RESET "\n ----------------------\n");
        table = table->next;
    }
}

int clientExistsInMemory(struct Memory **head_ref, int port)
{
    int clientExists = 0;
    struct Memory *exist = *head_ref;
    struct Member *member = exist->sharedBy;
    while (member != NULL)
    {
        if (member->data == port)
            return 1;
        member = member->next;
    }
    return 0;
}

void pushToTable(struct Memory **head_ref, int memID, int port, int socket)
{
    /* 
    if the memory is new then create a new table and push the new member, reurn "1" on sucess
    else memory exists before, then call addNewMember function to push the member to this memory
    return -4 if the client already in the memroy
    */
    struct Memory *memory = memoryExists(head_ref, memID);
    int memExists = 0;
    int clienInMemory = 0;

    if (memory != NULL)
    {
        memExists = 1;
        clienInMemory = clientExistsInMemory(&memory, port);
    }

    if (memExists == 0)
    {
        struct Member *new_member = (struct Member *)malloc(sizeof(struct Member));
        new_member->data = port;
        new_member->next = NULL;

        struct Memory *newTable = (struct Memory *)malloc(sizeof(struct Memory));
        newTable->memoryID = memID;
        newTable->sharedBy = new_member;
        newTable->lockedBy = 0;
        newTable->next = (*head_ref);
        (*head_ref) = newTable;
        send(socket, "1", sizeof("1"), 1024);
    }
    else
    {
        if (clienInMemory == 1)
        {
            // //error
            send(socket, "-4", sizeof("-4"), 0);
        }
        else
        {
            char memberss[1024];
            printf("the memory exists before\n");
            addNewMember(&memory->sharedBy, port);
            sendMembersToClientAsString(&memory->sharedBy, socket, memID);
        }
    }
}

///////////////////////////////////////// Main Operations ////////////////////////////////////////

void createOrAddToSharedMemoryOperation(struct Client client, int memId, int socket)
{
    char *response;
    printf("type one request\n");
    pushToTable(&tableHead, memId, client.portNumber, socket);
    //copy memory and send it to client

    // update other clients with changes
    struct Change *changes = createChangeObject(SHAREDBY_CHANGE, memId, "Nothing Changed!", -1, client.portNumber, -1);
    // // find clients and update them
    // struct Memory *memory = memoryExists(&tableHead, memId);
    // struct Member *memberPtr = memory->sharedBy;
    // while (memberPtr != NULL)
    // {
    //     notifyUpdates(changes, memberPtr->data);
    //     memberPtr = memberPtr->next;
    // }

    printTable(tableHead);
}

void readOperation(struct Request request, int socket)
{
    /*
    return -1 when the memory doesn't exist
            -2 when the client is not member of the memry
            -3 when the memory is locked by another
            [content] when sucess
    */
    char sendBuffer[100];
    struct Memory *memory = memoryExists(&tableHead, request.memId);
    int memExists = 0;
    int clienInMemory = 0;

    if (memory != NULL)
    {
        memExists = 1;
        clienInMemory = clientExistsInMemory(&memory, request.portNumber);
    }

    if (memExists == 0)
    {
        // //error
        sprintf(sendBuffer, "%d", -1);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
    }
    else if (clienInMemory == 0)
    {
        // //error
        sprintf(sendBuffer, "%d", -2);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
    }
    else
    {
        if (memory->lockedBy == 0 || memory->lockedBy == request.portNumber)
        {
            // tell client that you locked it
            send(socket, memory->content, sizeof(memory->content), 0);
        }
        else
        {
            send(socket, "-3", sizeof("-3"), 0);
        }
    }
}

void writeOperation(struct Request request, int socket)
{
    /*
    return -1 when the memory doesn't exist then end
            -2 when the client is not member of the memry then end
            1 so the client can write then server add new data to memroy and return index of new data
    */
    char sendBuffer[100];
    char readBuffer[1024];
    struct Memory *memory = memoryExists(&tableHead, request.memId);
    int memExists = 0;
    int clienInMemory = 0;

    if (memory != NULL)
    {
        memExists = 1;
        clienInMemory = clientExistsInMemory(&memory, request.portNumber);
    }

    if (memExists == 0)
    {
        // //error
        sprintf(sendBuffer, "%d", -1);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
    }
    else if (clienInMemory == 0)
    {
        // //error
        sprintf(sendBuffer, "%d", -2);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
    }
    else
    {
        send(socket, "1", sizeof("1"), 0);
        //wait the client to send data to be written
        read(socket, readBuffer, 1024);
        printf("******[%s]******* data received\n", readBuffer);
        int newDataIndex = (int)strlen(memory->content);
        sprintf(sendBuffer, "%d", newDataIndex); //index of new data
        strcat(memory->content, readBuffer);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);

        // update other clients with changes
        struct Change *changes = createChangeObject(CONTENT_CHANGE, memory->memoryID, readBuffer, newDataIndex, -1, -1);
        // find clients and update them
        // struct Member *memberPtr = memory->sharedBy;
        // while (memberPtr != NULL)
        // {
        //     notifyUpdates(changes, memberPtr->data);
        //             memberPtr = memberPtr->next;

        // }

        printTable(tableHead);
    }
}

void copyOperation(struct Request request, int socket)
{
    /*
    return -1 when the memory doesn't exist
            -2 when the client is not member of the memry
            -3 when the memory is locked by another
            [content] when sucess
    */
    char sendBuffer[100];
    struct Memory *memory = memoryExists(&tableHead, request.memId);
    int memExists = 0;
    int clienInMemory = 0;

    if (memory != NULL)
    {
        memExists = 1;
        clienInMemory = clientExistsInMemory(&memory, request.portNumber);
    }

    if (memExists == 0)
    {
        // //error
        sprintf(sendBuffer, "%d", -1);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
    }
    else if (clienInMemory == 0)
    {
        // //error
        sprintf(sendBuffer, "%d", -2);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
    }
    else
    {
        if (memory->lockedBy == 0 || memory->lockedBy == request.portNumber)
        {
            // tell client that you locked it
            send(socket, memory->content, sizeof(memory->content), 0);
        }
        else
        {
            send(socket, "-3", sizeof("-3"), 0);
        }
    }
}

void lockOperation(struct Request request, int socket)
{
    /*
    return -1 when the memory doesn't exist
            -2 when the client is not member of the memry
            client prot number when sucess
    */
    printf("2\n");
    char sendBuffer[100];
    struct Memory *memory = memoryExists(&tableHead, request.memId);
    int memExists = 0;
    int clienInMemory = 0;

    if (memory != NULL)
    {
        memExists = 1;
        clienInMemory = clientExistsInMemory(&memory, request.portNumber);
    }

    if (memExists == 0)
    {
        // //error
        sprintf(sendBuffer, "%d", -1);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
    }
    else if (clienInMemory == 0)
    {
        // //error
        sprintf(sendBuffer, "%d", -2);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
    }
    else
    {
        if (memory->lockedBy == 0)
        {
            memory->lockedBy = request.portNumber;
            // tell client that you locked it
            sprintf(sendBuffer, "%d", memory->lockedBy);
            send(socket, sendBuffer, sizeof(sendBuffer), 0);
        }
        else
        {
            while (memory->lockedBy != 0 && memory->lockedBy != request.portNumber)
                ; //print waiting once
            memory->lockedBy = request.portNumber;
            // tell client that you locked it
            sprintf(sendBuffer, "%d", memory->lockedBy);
            send(socket, sendBuffer, sizeof(sendBuffer), 0);
        }

        // update other clients with changes
        struct Change *changes = createChangeObject(LOCK_CHANGE, memory->memoryID, "Nothing Changed!", -1, -1, request.portNumber);
        // printf("Type: %d\nMem: %d\nContent: %s\nLock: %d\n", changes->type, changes->memId, changes->content, changes->lockedBy);
        // find clients and update them
        struct Member *memberPtr = memory->sharedBy;
        while (memberPtr != NULL)
        {
            // printf("Member: %d\n", memberPtr->data);

            notifyUpdates(changes, memberPtr->data);
            memberPtr = memberPtr->next;

            // }
        }
        printTable(tableHead);
    }
}

    void unLockOperation(struct Request request, int socket)
    {
        /*
    return -1 when the memory doesn't exist
            -2 when the client is not member of the memry
            0 when sucess
    */
        char sendBuffer[100];
        struct Memory *memory = memoryExists(&tableHead, request.memId);
        int memExists = 0;
        int clienInMemory = 0;

        if (memory != NULL)
        {
            memExists = 1;
            clienInMemory = clientExistsInMemory(&memory, request.portNumber);
        }

        if (memExists == 0)
        {
            // //error
            sprintf(sendBuffer, "%d", -1);
            send(socket, sendBuffer, sizeof(sendBuffer), 0);
        }
        else if (clienInMemory == 0)
        {
            // //error
            sprintf(sendBuffer, "%d", -2);
            send(socket, sendBuffer, sizeof(sendBuffer), 0);
        }
        else
        {
            if (memory->lockedBy == 0 || memory->lockedBy == request.portNumber)
            {
                memory->lockedBy = 0;
                // tell client that you unlocked it
            }
            sprintf(sendBuffer, "%d", memory->lockedBy);
            send(socket, sendBuffer, sizeof(sendBuffer), 0);

            // update other clients with changes
            struct Change *changes = createChangeObject(UNLOCK_CHANGE, memory->memoryID, "Nothing Changed!", -1, -1, 0);
            // find clients and update them
            // struct Member *memberPtr = memory->sharedBy;
            // while (memberPtr != NULL)
            // {
            //     notifyUpdates(changes, memberPtr->data);
            //             memberPtr = memberPtr->next;
            // }
        }
        printTable(tableHead);
    }

    ///////////////////////////////////////// Server Handler  ////////////////////////////////////////

    struct Request getRequestDetails(char *buffer)
    {

        char *token;
        struct Request *request = (struct Request *)malloc(sizeof(struct Request));
        token = strtok(buffer, ":");
        sscanf(token, "%d", &request->portNumber);
        token = strtok(NULL, ":");
        sscanf(token, "%d", &request->memId);
        token = strtok(NULL, ":");
        sscanf(token, "%d", &request->type);

        return *request;
    }

    void serverHandler(void *socket)
    {
        int new_socket = *((int *)socket);
        char buffer[1024];
        read(new_socket, buffer, 1024);
        // printf("I read buffer [%s]\n", buffer);
        struct Request request = getRequestDetails(buffer);
        // printf("I reeived a message type %d of memId %d from %d\n", request.type, request.memId, request.portNumber);
        struct Client *client = (struct Client *)malloc(sizeof(struct Client));
        client->portNumber = request.portNumber;

        if (request.type == CREATE)
        {
            createOrAddToSharedMemoryOperation(*client, request.memId, new_socket);
        }
        else if (request.type == READ)
        {
            readOperation(request, new_socket);
        }
        else if (request.type == WRITE)
        {
            writeOperation(request, new_socket);
        }
        else if (request.type == COPY)
        {
            copyOperation(request, new_socket);
        }
        else if (request.type == LOCK)
        {
            lockOperation(request, new_socket);
        }
        else if (request.type == UNLOCK)
        {
            unLockOperation(request, new_socket);
        }
        else
        {
            printf("not defined type");
        }
        // send(new_socket, "1", sizeof("1"), 0);
        //tableHead = (struct MemoryTable*)malloc(sizeof(struct MemoryTable));
    }

    ///////////////////////////////////////// Main function ////////////////////////////////////////
    int main(void)
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
        serv_adr.sin_port = htons(SERVERPORT);        //initiating port number

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
            printf("start while loop\n");
            // clnt_len = sizeof(clnt_adr);
            if ((new_sock = accept(orig_sock, (struct sockaddr *)&clnt_adr, &clnt_len)) < 0)
            {
                perror("accept error");
                close(orig_sock);
                exit(4);
            }
            pthread_t thread;
            pthread_create(&thread, NULL, (void *)serverHandler, (void *)&new_sock);
            // pthread_join(thread, NULL);
            printf("after thread\n");
        }

        return 0;
    }
    //////////////////////////////////////////////////////////////fgfg//////////////////////////////////