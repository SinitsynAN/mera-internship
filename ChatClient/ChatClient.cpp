#include "ChatClient.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

ChatClient::ChatClient()
{
    sender = -1;
    address = 0;
    port = 0;
    isSocketOpened = false;
}

ChatClient::~ChatClient()
{
    if (isSocketOpened)
        close(sender);
}

bool ChatClient::InitSocket(unsigned long targetAddress, unsigned short targetPort)
{
    if (!isSocketOpened) {
        sender = socket(AF_INET, SOCK_DGRAM, 0);
        if (sender < 0)
        {
            perror("Socket isn't created\n");
            return isSocketOpened;
        }

        socketAddress.sin_family = AF_INET;
        socketAddress.sin_port = htons(targetPort);
        socketAddress.sin_addr.s_addr = htonl(targetAddress);
        isSocketOpened = true;
    }
    else {
        perror("Socket has been already opened\n");
    }
    
    return isSocketOpened;
}

void ChatClient::Send(char message[])
{
    int size = 0;
    while (message[size++] != '\0')
        ;
    if (isSocketOpened)
        sendto(sender, message, size, 0, 
            (struct sockaddr *) &socketAddress, sizeof(socketAddress));
}