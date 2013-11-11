#include "ChatClient.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

ChatClient::ChatClient()
{
    sender = -1;
    isSocketOpened = false;
    clientNickname = new char[7];
    strcpy(clientNickname, "Anonym");
}

ChatClient::~ChatClient()
{
    close(sender);
    delete [] clientNickname;
}

void ChatClient::SetNickname(char *nickname)
{
    delete [] clientNickname;
    
    int len = strlen(nickname);
    clientNickname = new char[len + 1];
    strcpy(clientNickname, nickname);
    if (clientNickname[len - 1] == '\n')
        clientNickname[len - 1] = '\0';
}

void ChatClient::InitSocket(char *targetAddress, unsigned short targetPort)
{
    if (!isSocketOpened) {
        sender = socket(AF_INET, SOCK_DGRAM, 0);
        if (sender < 0) {
            perror("Socket isn't created");
            return;
        }

        senderAddress.sin_family = AF_INET;
        senderAddress.sin_port = htons(targetPort);
        
        if (inet_pton(AF_INET, targetAddress, &senderAddress.sin_addr) <= 0) {
            perror("Invalid IP-address");
            return ;
        }
        
        isSocketOpened = true;
    }
    else {
        perror("Socket has been already opened");
    }
}

void ChatClient::Send(char *message)
{
    int msgSize = strlen(message) + 1;
    if (isSocketOpened && msgSize > 1) {
        sendto(sender, clientNickname, strlen(clientNickname) + 1, 0, (struct sockaddr *) &senderAddress, sizeof(senderAddress));
        sendto(sender, message, msgSize, 0, (struct sockaddr *) &senderAddress, sizeof(senderAddress));
    }
}