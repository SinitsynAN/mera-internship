#ifndef CHATSERVER_H
#define	CHATSERVER_H

#define BUFFER_SIZE 1024

#include <netinet/in.h>

class ChatServer
{
private:
    int listener;
    struct sockaddr_in socketAddress;
    char buffer[BUFFER_SIZE];
    int bytesRead;
    bool isSocketValid;
    bool isStarted;
    
public:
    ChatServer();
    bool InitSocket(unsigned short port);
    void Start();
};

#endif