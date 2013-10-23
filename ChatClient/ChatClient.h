#ifndef CHATCLIENT_H
#define	CHATCLIENT_H

#include <netinet/in.h>

class ChatClient
{
private:
    int sender;
    unsigned long address;
    unsigned short port;
    struct sockaddr_in socketAddress;
    bool isSocketOpened;
    
public:
    ChatClient();
    ~ChatClient();
    bool InitSocket(unsigned long targetAddress, unsigned short targetPort);
    void Send(char message[]);
};

#endif