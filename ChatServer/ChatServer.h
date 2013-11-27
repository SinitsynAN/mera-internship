#ifndef CHATSERVER_H
#define	CHATSERVER_H

#define BUFFER_SIZE 1024
#define SELECT_TIMEOUT_SEC 10

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
    
    int broadcast;
    struct sockaddr_in broadcastAddress;
    
    bool isStarted;
    
    char buffer[BUFFER_SIZE];
    
    void BindSocketForSupervisor();
    void ConnectSocketForSupervisor();
    
    void Work();
    
public:
    ChatServer();
    ~ChatServer();
    void InitSocketForSupervisor(char *serverCommFilepath, char *supervisorCommFilepath);
    void InitSocketForClients(unsigned short port);
    void Start();
};

#endif