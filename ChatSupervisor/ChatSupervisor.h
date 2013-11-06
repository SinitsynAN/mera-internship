#ifndef CHATSUPERVISOR_H
#define	CHATSUPERVISOR_H

#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 10
#define CONNECTION_TRIES_COUNT 5

#include <sys/un.h>

class ChatSupervisor
{
private:
    int svSocket;
    struct sockaddr_un supervisorAddress;
    struct sockaddr_un serverAddress;
    bool isSocketBinded;
    bool isSocketConnected;
    
    bool isStarted;
    
    bool isServerAvailable;
    int connectionTriesCount;
    
    char buffer[BUFFER_SIZE];
    
    void BindSocket();
    void ConnectSocket();
    
public:
    ChatSupervisor();
    ~ChatSupervisor();
    void InitSocket(char supervisorCommFilepath[], char serverCommFilepath[]);
    void Start();
};

#endif