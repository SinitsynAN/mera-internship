#ifndef CHATCLIENT_H
#define	CHATCLIENT_H

#include <netinet/in.h>

class ChatClient
{
private:
    int sender;
    struct sockaddr_in socketAddress;
    bool isSocketOpened;
    
public:
    ChatClient();
    ~ChatClient();
    bool InitSocket(char targetAddress[], unsigned short targetPort);
    void Send(char message[]);
};

#endif