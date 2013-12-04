#ifndef CHATSERVER_H
#define	CHATSERVER_H

#define BUFFER_SIZE 1024
#define SELECT_TIMEOUT_SEC 10
#define MAX_CLIENT_COUNT 100

#include <netinet/in.h>
#include <sys/un.h>

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
    
    struct sockaddr_in clientAddresses[MAX_CLIENT_COUNT];
    int clientCount;
    
    bool isStarted;
    
    char buffer[BUFFER_SIZE];
    
    void BindSocketForSupervisor();
    void ConnectSocketForSupervisor();
    
    void Work();
    
    bool CompareAddresses(struct sockaddr_in firstAddress, struct sockaddr_in secondAddress);
    void AddSend(struct sockaddr_in clientAddress, int messageLength);
    
public:
    ChatServer();
    ~ChatServer();
    void InitSocketForSupervisor(char *serverCommFilepath, char *supervisorCommFilepath);
    void InitSocketForClients(char *filepath);
    void Start();
};

#endif