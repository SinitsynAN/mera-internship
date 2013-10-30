#include "ChatServer.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
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
        
        //select-changes begin here
        fcntl(listener, F_SETFL, O_NONBLOCK);
        //select-changes end here

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
                //select-changes begin here
                fd_set readset;
                FD_ZERO(&readset);
                FD_SET(listener, &readset);
                
                timeval timeout;
                timeout.tv_sec = 15;
                timeout.tv_usec = 0;
                
                if (select(listener + 1, &readset, NULL, NULL, &timeout) <= 0) {
                    perror("Problems with select\n");
                    break;
                }
                
                if (FD_ISSET(listener, &readset)) {
                    bytesRead = recvfrom(listener, buffer, BUFFER_SIZE, 0, NULL, NULL);
                    buffer[bytesRead] = '\0';
                    printf("%s", buffer);
                }
                //select-changes end here
                
                /* without  select (blocking)
                 * printf("Waiting a message...\n > ");
                bytesRead = recvfrom(listener, buffer, BUFFER_SIZE, 0, NULL, NULL);
                buffer[bytesRead] = '\0';
                printf("%s", buffer);
                printf("Done\n");*/
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