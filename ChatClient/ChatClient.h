#ifndef CHATCLIENT_H
#define	CHATCLIENT_H

#include <netinet/in.h>

class ChatClient
{
private:
    int sender;
    struct sockaddr_in senderAddress;
    bool isSocketOpened;
    char *clientNickname;
    
public:
    ChatClient();
    ~ChatClient();
    void SetNickname(char *nickname);
    void InitSocket(char *targetAddress, unsigned short targetPort);
    void Send(char *message);
};

#endif