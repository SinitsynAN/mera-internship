#ifndef CHATSERVER_H
#define	CHATSERVER_H

#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 10

#include <sys/un.h>
#include <netinet/in.h>

class ChatServer
{
private:
    int sSocket;
    struct sockaddr_un serverAddress;
    struct sockaddr_un supervisorAddress;
    bool isSSocketBinded;
    bool isSSocketConnected;
    
    int clientSocket;
    struct sockaddr_in clientSocketAddress;
    bool isCSocketBinded;
    
    bool isStarted;
    
    char buffer[BUFFER_SIZE];
    
    void BindSSocket();
    void ConnectSSocket();
    
public:
    ChatServer();
    ~ChatServer();
    void InitSSocket(char serverCommFilepath[], char supervisorCommFilepath[]);
    void InitClientSocket(unsigned short port);
    void Start();
};

#endif