#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "ChatServer.h"

ChatServer::ChatServer()
{
    socketForSupervisor = -1;
    isSocketForSupervisorBinded = false;
    isSocketForSupervisorConnected = false;
    
    socketForClients = -1;
    isSocketForClientsBinded = false;
    
    isStarted = false;
    
    for (int i = 0; i < BUFFER_SIZE; i++)
        buffer[i] = 0;
}

ChatServer::~ChatServer()
{
    close(socketForSupervisor);
    close(socketForClients);
}

void ChatServer::InitSocketForSupervisor(char *serverCommFilepath, char *supervisorCommFilepath)
{
    if (!isStarted) {
        socketForSupervisor = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (socketForSupervisor < 0) {
            perror("socketForSupervisor isn't created");
        }
        fcntl(socketForSupervisor, F_SETFL, O_NONBLOCK);
        
        serverAddress.sun_family = AF_UNIX;
        strcpy(serverAddress.sun_path, serverCommFilepath);
        unlink(serverAddress.sun_path);
        
        BindSocketForSupervisor();
        
        supervisorAddress.sun_family = AF_UNIX;
        strcpy(supervisorAddress.sun_path, supervisorCommFilepath);
        
        ConnectSocketForSupervisor();
    }
}

void ChatServer::BindSocketForSupervisor()
{
   if (bind(socketForSupervisor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("Problems with socketForSupervisor binding");
    }
    isSocketForSupervisorBinded = true; 
}

void ChatServer::ConnectSocketForSupervisor()
{
    if (connect(socketForSupervisor, (struct sockaddr *) &supervisorAddress, sizeof(supervisorAddress)) < 0) {
        perror("Problems with socketForSupervisor connecting");
    }
    isSocketForSupervisorConnected = true;
}

void ChatServer::InitSocketForClients(unsigned short port)
{
    if (!isStarted) {    
        socketForClients = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketForClients < 0) {
            perror("clientSocket isn't created");
        }
        fcntl(socketForClients, F_SETFL, O_NONBLOCK);

        clientSocketAddress.sin_family = AF_INET;
        clientSocketAddress.sin_port = htons(port);
        clientSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind(socketForClients, (struct sockaddr *) &clientSocketAddress, sizeof(clientSocketAddress)) < 0) {
            perror("Problems with clientSocket binding");
        }
        isSocketForClientsBinded = true;
    }
}

void ChatServer::Start()
{
    if (!isStarted) {
        if (isSocketForSupervisorBinded && isSocketForSupervisorConnected && isSocketForClientsBinded) {
            printf("Server is successfully started\n");
            send(socketForSupervisor, "I've started", 13, 0);
            Work();
        }
        else {
            perror("Problems with connections");
        }
    }
    else {
        perror("Server has been already started");
    }
}

void ChatServer::Work()
{
    int bytesReceived = 0;
    while(1) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(socketForSupervisor, &readSet);
        FD_SET(socketForClients, &readSet);

        timeval timeout;
        timeout.tv_sec = SELECT_TIMEOUT_SEC;
        timeout.tv_usec = 0;

        int maxFd = socketForSupervisor > socketForClients ? socketForSupervisor : socketForClients;
        int selectResult = select(maxFd + 1, &readSet, NULL, NULL, &timeout);
        if (selectResult < 0) {
            perror("Problems with select");
            break;
        }
        else if (selectResult == 0) {
            // timeout is expired
            continue;
        }

        if (FD_ISSET(socketForSupervisor, &readSet)) {
            bytesReceived = recv(socketForSupervisor, buffer, BUFFER_SIZE, 0);
            buffer[bytesReceived] = '\0';
            printf(">[S]: %s\n", buffer);

            send(socketForSupervisor, "I'm alive", 10, 0);
        }

        if (FD_ISSET(socketForClients, &readSet)) {
            bytesReceived = recvfrom(socketForClients, buffer, BUFFER_SIZE, 0, NULL, NULL);
            buffer[bytesReceived] = '\0';
            printf(">[S]: %s\n", buffer);
        }
    }
}