#include "ChatSupervisor.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

ChatSupervisor::ChatSupervisor()
{
    svSocket = -1;
    isSocketBinded = false;
    isSocketConnected = false;
    
    isStarted = false;
    
    isServerAvailable = false;
    connectionTriesCount = CONNECTION_TRIES_COUNT;
    
    for (int i = 0; i < BUFFER_SIZE; i ++)
        buffer[i] = 0;
}

ChatSupervisor::~ChatSupervisor()
{
    close(svSocket);
}

void ChatSupervisor::InitSocket(char supervisorCommFilepath[], char serverCommFilepath[])
{
    if (!isStarted) {
        svSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (svSocket < 0) {
            perror("svSocket isn't created");
        }
        fcntl(svSocket, F_SETFL, O_NONBLOCK);
        
        supervisorAddress.sun_family = AF_UNIX;
        strcpy(supervisorAddress.sun_path, supervisorCommFilepath);
        unlink(supervisorAddress.sun_path);
        
        BindSocket();
        
        serverAddress.sun_family = AF_UNIX;
        strcpy(serverAddress.sun_path, serverCommFilepath);
    }
}

void ChatSupervisor::BindSocket()
{
   if (bind(svSocket, (struct sockaddr *) &supervisorAddress, sizeof(supervisorAddress)) < 0) {
            perror("Problems with svSocket binding");
        }
        isSocketBinded = true; 
}

void ChatSupervisor::ConnectSocket()
{
    if (connect(svSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("Problems with svSocket connecting");
    }
    isSocketConnected = true;
}

void ChatSupervisor::Start()
{
    if (!isStarted) {
        if (isSocketBinded) {
            printf("Supervisor is successfully started\n");
            int bytesReceived = 0;
            while(1) {
                fd_set readSet;
                FD_ZERO(&readSet);
                FD_SET(svSocket, &readSet);
                
                fd_set writeSet;
                FD_ZERO(&writeSet);
                FD_SET(svSocket, &writeSet);
                
                timeval timeout;
                timeout.tv_sec = TIMEOUT_SEC;
                timeout.tv_usec = 0;
                
                int maxFd = svSocket;
                int selectResult = select(maxFd + 1, &readSet, &writeSet, NULL, &timeout);
                if (selectResult < 0) {
                    perror("Problems with select");
                    break;
                }
                else if (selectResult == 0) {
                    printf("Timeout is expired\n");
                    continue;
                }

                if (FD_ISSET(svSocket, &writeSet)) {
                    if (isServerAvailable) {
                        sleep(5);
                        send(svSocket, "Are you alive?", 15, 0);
                    }
                }
                
                if (FD_ISSET(svSocket, &readSet)) {
                    bytesReceived = recv(svSocket, buffer, BUFFER_SIZE, 0);
                    buffer[bytesReceived] = '\0';
                    printf("%s\n", buffer);
                    fflush(stdout);
                    
                    if (!isServerAvailable && strcmp(buffer, "I started") == 0) {
                        isServerAvailable = true;
                        ConnectSocket();
                    }
                }
                
                if (!isServerAvailable) {
                    sleep(5);
                    if (connectionTriesCount > 0) {
                        connectionTriesCount--;
                        continue;
                    }
                    else {
                        printf("Server is unavailable\n");
                        break;
                    }
                }
            }
        }
        else {
            perror("svSocket isn't binded");
        }
    }
    else {
        perror("Server has been already started");
    }
}