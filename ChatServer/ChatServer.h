#ifndef CHATSERVER_H
#define	CHATSERVER_H

#define BUFFER_SIZE 1024
#define SELECT_TIMEOUT_SEC 10

#include <sys/un.h>
#include <netinet/in.h>

class ChatServer
{
private:
    int socketForServer;
    struct sockaddr_un serverAddress;
    struct sockaddr_un supervisorAddress;
    bool isSocketForServerBinded;
    bool isSocketForServerConnected;
    
    int socketForClients;
    struct sockaddr_in clientSocketAddress;
    bool isSocketForClientsBinded;
    
    bool isStarted;
    
    char buffer[BUFFER_SIZE];
    
    void BindSocketForServer();
    void ConnectSocketForServer();
    
    void Work();
    
public:
    ChatServer();
    ~ChatServer();
    void InitSocketForServer(char *serverCommFilepath, char *supervisorCommFilepath);
    void InitSocketForClients(unsigned short port);
    void Start();
};

#endif