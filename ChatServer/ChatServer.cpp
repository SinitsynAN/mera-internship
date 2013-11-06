#include "ChatServer.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

ChatServer::ChatServer()
{
    sSocket = -1;
    isSSocketBinded = false;
    isSSocketConnected = false;
    
    clientSocket = -1;
    isCSocketBinded = false;
    
    isStarted = false;
    
    for (int i = 0; i < BUFFER_SIZE; i++)
        buffer[i] = 0;
}

ChatServer::~ChatServer()
{
    close(sSocket);
    close(clientSocket);
}

void ChatServer::InitSSocket(char serverCommFilepath[], char supervisorCommFilepath[])
{
    if (!isStarted) {
        sSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (sSocket < 0) {
            perror("sSocket isn't created");
        }
        fcntl(sSocket, F_SETFL, O_NONBLOCK);
        
        serverAddress.sun_family = AF_UNIX;
        strcpy(serverAddress.sun_path, serverCommFilepath);
        unlink(serverAddress.sun_path);
        
        BindSSocket();
        
        supervisorAddress.sun_family = AF_UNIX;
        strcpy(supervisorAddress.sun_path, supervisorCommFilepath);
        
        ConnectSSocket();
    }
}

void ChatServer::BindSSocket()
{
   if (bind(sSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("Problems with svSocket binding");
    }
    isSSocketBinded = true; 
}

void ChatServer::ConnectSSocket()
{
    if (connect(sSocket, (struct sockaddr *) &supervisorAddress, sizeof(supervisorAddress)) < 0) {
        perror("Problems with svSocket connecting");
    }
    isSSocketConnected = true;
}

void ChatServer::InitClientSocket(unsigned short port)
{
    if (!isStarted) {    
        clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (clientSocket < 0) {
            perror("clientSocket isn't created");
        }
        fcntl(clientSocket, F_SETFL, O_NONBLOCK);

        clientSocketAddress.sin_family = AF_INET;
        clientSocketAddress.sin_port = htons(port);
        clientSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind(clientSocket, (struct sockaddr *) &clientSocketAddress, sizeof(clientSocketAddress)) < 0) {
            perror("Problems with clientSocket binding");
        }
        isCSocketBinded = true;
    }
}

void ChatServer::Start()
{
    if (!isStarted) {
        if (isSSocketBinded && isSSocketConnected && isCSocketBinded) {
            printf("Server is successfully started\n");
            send(sSocket, "I started", 10, 0);
            int bytesReceived = 0;
            while(1) {
                fd_set readSet;
                FD_ZERO(&readSet);
                FD_SET(sSocket, &readSet);
                FD_SET(clientSocket, &readSet);
                
                timeval timeout;
                timeout.tv_sec = TIMEOUT_SEC;
                timeout.tv_usec = 0;
                
                int maxFd = sSocket > clientSocket ? sSocket : clientSocket;
                int selectResult = select(maxFd + 1, &readSet, NULL, NULL, &timeout);
                if (selectResult < 0) {
                    perror("Problems with select");
                    break;
                }
                else if (selectResult == 0) {
                    printf("Timeout is expired\n");
                    continue;
                }
                
                if (FD_ISSET(sSocket, &readSet)) {
                    bytesReceived = recv(sSocket, buffer, BUFFER_SIZE, 0);
                    buffer[bytesReceived] = '\0';
                    printf("%s\n", buffer);
                    
                    send(sSocket, buffer, bytesReceived, 0);
                }
                
                if (FD_ISSET(clientSocket, &readSet)) {
                    bytesReceived = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, NULL, NULL);
                    buffer[bytesReceived] = '\0';
                    printf("%s\n", buffer);
                }
            }
        }
        else {
            perror("Problems with connections");
        }
    }
    else {
        perror("Server has been already started");
    }
}