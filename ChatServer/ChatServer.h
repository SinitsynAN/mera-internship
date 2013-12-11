#ifndef CHATSERVER_H
#define	CHATSERVER_H

#include "ClientInfo.h"
#include "SServiceMessages.h"

#include <Logger.h>

#include <netinet/in.h>
#include <sys/un.h>

#define MAX_CLIENT_COUNT 100

#define SELECT_TIMEOUT_SEC 5

#define BUFFER_SIZE 1024

#define LOG_FILEPATH "/tmp/s_log"

class ChatServer
{
private:
    int socketForSupervisor;
    struct sockaddr_un serverAddress;
    struct sockaddr_un supervisorAddress;
    bool isSocketForSupervisorBinded;
    bool isSocketForSupervisorConnected;
    
    int socketForClients;
    struct sockaddr_in clientSocketAddress;
    bool isSocketForClientsBinded;
    
    ClientInfo clients[MAX_CLIENT_COUNT];
    int clientCount;
    
    bool isStarted;
    
    char buffer[BUFFER_SIZE];
    
    SServiceMessages serviceMessages;
    
    Logger *logger;
    
    void BindSocketForSupervisor();
    void ConnectSocketForSupervisor();
    
    void Work();
    
    bool CompareAddresses(struct sockaddr_in firstAddress, struct sockaddr_in secondAddress);
    int FindClient(struct sockaddr_in address);
    void RemoveClient(int number);
    
    void AddSend(struct sockaddr_in clientAddress, int messageLength);
    
public:
    ChatServer();
    ~ChatServer();
    
    void InitSocketForSupervisor(char *serverCommFilepath, char *supervisorCommFilepath);
    void InitSocketForClients(char *filepath);
    void Start();
};

#endif