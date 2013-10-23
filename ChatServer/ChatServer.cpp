#include "ChatServer.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

ChatServer::ChatServer()
{
    listener = -1;
    for (int i = 0; i < BUFFER_SIZE; i++)
        buffer[i] = 0;
    bytesRead = 0;
    isSocketValid = false;
    isStarted = false;
}

bool ChatServer::InitSocket(unsigned short port)
{
    if (!isStarted) {
        isSocketValid = false;

        listener = socket(AF_INET, SOCK_DGRAM, 0);
        if (listener < 0) {
            perror("Socket isn't created\n");
            return isSocketValid;
        }

        socketAddress.sin_family = AF_INET;
        socketAddress.sin_port = htons(port);
        socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(listener, (struct sockaddr *) &socketAddress, sizeof(socketAddress)) < 0) {
            perror("Problems with socket binding\n");
            return isSocketValid;
        }

        isSocketValid = true;
    }
    
    return isSocketValid;
}

void ChatServer::Start()
{
    if (!isStarted) {
        if (isSocketValid) {
            printf("Server is successfully started\n");
            while(1) {
                bytesRead = recvfrom(listener, buffer, BUFFER_SIZE, 0, NULL, NULL);
                buffer[bytesRead] = '\0';
                printf("%s", buffer);
            }
        }
        else {
            perror("Problems with socket\n");
        }
    }
    else {
        perror("Server has been already started\n");
    }
}