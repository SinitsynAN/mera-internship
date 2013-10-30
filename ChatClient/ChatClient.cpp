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
}

ChatClient::~ChatClient()
{
    if (isSocketOpened)
        close(sender);
}

bool ChatClient::InitSocket(char targetAddress[], unsigned short targetPort)
{
    if (!isSocketOpened) {
        sender = socket(AF_INET, SOCK_DGRAM, 0);
        if (sender < 0) {
            perror("Socket isn't created\n");
            return isSocketOpened;
        }

        socketAddress.sin_family = AF_INET;
        socketAddress.sin_port = htons(targetPort);
        
        if (inet_pton(AF_INET, targetAddress, &socketAddress.sin_addr) <= 0) {
            perror("Invalid IP-address\n");
            return isSocketOpened;
        }
        
        isSocketOpened = true;
    }
    else {
        perror("Socket has been already opened\n");
    }
    
    return isSocketOpened;
}

void ChatClient::Send(char message[])
{
    int size = strlen(message) + 1;
    if (isSocketOpened && size > 1)
        sendto(sender, message, size, 0, 
            (struct sockaddr *) &socketAddress, sizeof(socketAddress));
}