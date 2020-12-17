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
//inform client about lock (when create memory)


#include "utils.c"

///////////////////////////////////////// MemoryTable ////////////////////////////////////////

struct Memory *tableHead = NULL;

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
    printf("start print \n........... \n");
    while (table != NULL)
    {
        printf(" %d => ", table->memoryID);
        printf("locked by: %d\n", table->lockedBy);
        struct Member *mem = table->sharedBy;
        while (mem != NULL)
        {
            printf(" %d ,", mem->data);
            mem = mem->next;
        }
        printf("\n \n");
        table = table->next;
    }
}

struct Memory *momoryExists(struct Memory **head_ref, int memID)
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

void pushToTable(struct Memory **head_ref, int memID, int port, int socket)
{
    /* 
    if the memory is new then create a new table and push the new member, reurn "1" on sucess
    else memory exists before, then call addNewMember function to push the member to this memory
    */
    struct Memory *exist = momoryExists(head_ref, memID);
    int memExists = 0;
    if (exist != NULL)
        memExists = 1;

    if (memExists == 1)
    {
        char memberss[1024];
        printf("the memory exists before\n");
        addNewMember(&exist->sharedBy, port);
        sendMembersToClientAsString(&exist->sharedBy, socket, memID);
    }
    else
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
}

///////////////////////////////////////// Main Operations ////////////////////////////////////////

void createOrAddToSharedMemoryOperation(struct Client client, int memId, int socket)
{
    char *response;
    printf("type one request\n");
    pushToTable(&tableHead, memId, client.portNumber, socket);
    //copy memory and send it to client
    printTable(tableHead);
}

void readOperation(struct Request request, int socket)
{
    char sendBuffer[100];
    struct Memory *memory = momoryExists(&tableHead,request.memId);
    int memExists = 0;
    if (memory != NULL)
        memExists = 1;

    if (memExists == 1)
    {
        if(memory->lockedBy == 0 || memory->lockedBy ==request.portNumber) {
           // tell client that you locked it 
           send(socket, memory->content, sizeof(memory->content), 0);
        }
        else{
            send(socket, "-1", sizeof("-1"), 0);
        }
    } else {
        //error
        send(socket, "-1", sizeof("-1"), 0);
    }
}

void writeOperation()
{
}

void copyOperation(struct Request request, int socket)
{
    char sendBuffer[100];
    struct Memory *memory = momoryExists(&tableHead,request.memId);
    int memExists = 0;
    if (memory != NULL)
        memExists = 1;

    if (memExists == 1)
    {
        if(memory->lockedBy == 0 || memory->lockedBy ==request.portNumber) {
           // tell client that you locked it 
           send(socket, memory->content, sizeof(memory->content), 0);
        }
        else{
            send(socket, "-1", sizeof("-1"), 0);
        }
    } else {
        //error
        send(socket, "-1", sizeof("-1"), 0);
    }
}

void lockOperation(struct Request request, int socket)
{ 
    printf("2\n");
    char sendBuffer[100];
    struct Memory *memory = momoryExists(&tableHead,request.memId);
    int memExists = 0;
    if (memory != NULL)
        memExists = 1;

    if (memExists == 1)
    {
        if(memory->lockedBy == 0) {
            memory->lockedBy = request.portNumber;
           // tell client that you locked it 
           sprintf(sendBuffer, "%d", memory->lockedBy);
            send(socket, sendBuffer, sizeof(sendBuffer), 0);
        }
        else{
            while(memory->lockedBy !=0 && memory->lockedBy != request.portNumber);
            memory->lockedBy = request.portNumber;
            // tell client that you locked it 
            sprintf(sendBuffer, "%d", memory->lockedBy);
            send(socket, sendBuffer, sizeof(sendBuffer), 0);
        }
        
    } else {
        //error
        sprintf(sendBuffer, "%d",0);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
    }
    printTable(tableHead);


}

void unLockOperation(struct Request request, int socket)
{
    char sendBuffer[100];
    struct Memory *memory = momoryExists(&tableHead,request.memId);
    int memExists = 0;
    if (memory != NULL)
        memExists = 1;

    if (memExists == 1)
    {
        if(memory->lockedBy == 0 || memory->lockedBy == request.portNumber) {
            memory->lockedBy = 0;
           // tell client that you unlocked it 
        }
        sprintf(sendBuffer, "%d", memory->lockedBy);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
    } else {
        //error
        sprintf(sendBuffer, "%d", -1);
        send(socket, sendBuffer, sizeof(sendBuffer), 0);
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
        writeOperation();
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

    if (listen(orig_sock, 5) < 0)
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