#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
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
    
    memset(buffer, '\0', BUFFER_SIZE * sizeof(char));
    
    memset(clientAddresses, 0, MAX_CLIENT_COUNT * sizeof(struct sockaddr_in));
    clientCount = 0;
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

void ChatServer::InitSocketForClients(char *filepath)
{
    if (!isStarted) {
        FILE *config = fopen(filepath, "r");
        if (config == NULL) {
            perror("Can't open config file");
            return;
        }

        char port[10] = {0};
        fscanf(config, "%s", port);
        fclose(config);
        
        socketForClients = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketForClients < 0) {
            perror("clientSocket isn't created");
        }
        fcntl(socketForClients, F_SETFL, O_NONBLOCK);

        clientSocketAddress.sin_family = AF_INET;
        clientSocketAddress.sin_port = htons(atoi(port));
        clientSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind(socketForClients, (struct sockaddr *) &clientSocketAddress, sizeof(clientSocketAddress)) < 0) {
            perror("Problems with clientSocket binding");
            return;
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

bool ChatServer::CompareAddresses(struct sockaddr_in firstAddress, struct sockaddr_in secondAddress)
{
    if (firstAddress.sin_family != secondAddress.sin_family)
        return false;
    if (firstAddress.sin_addr.s_addr != secondAddress.sin_addr.s_addr)
        return false;
    if (firstAddress.sin_port != secondAddress.sin_port)
        return false;
    return true;
}

void ChatServer::AddSend(struct sockaddr_in clientAddress, int messageLength)
{
    bool doesExist = false;
    for (int i = 0; i < clientCount; i++) {
        if (CompareAddresses(clientAddress, clientAddresses[i])) {
            doesExist = true;
            sendto(socketForClients, "MSG_RCVD", 9, 0, (struct sockaddr *) &(clientAddresses[i]), sizeof(clientAddresses[i]));
        }
        sendto(socketForClients, buffer, messageLength, 0, (struct sockaddr *) &(clientAddresses[i]), sizeof(clientAddresses[i]));
    }
    if (!doesExist) {
        if (clientCount < MAX_CLIENT_COUNT) {
            clientAddresses[clientCount] = clientAddress;
            sendto(socketForClients, "MSG_RCVD", 9, 0, (struct sockaddr *) &(clientAddresses[clientCount]), sizeof(clientAddresses[clientCount]));
            sendto(socketForClients, buffer, messageLength, 0, (struct sockaddr *) &(clientAddresses[clientCount]), sizeof(clientAddresses[clientCount]));
            clientCount++;
        }
        else {
            perror("Can't add a client");
        }
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
            struct sockaddr_in clientAddress;
            socklen_t len = sizeof(clientAddress);
            
            bytesReceived = recvfrom(socketForClients, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &clientAddress, &len);
            buffer[bytesReceived] = '\0';
            printf(">[S]: %s\n", buffer);
            
            AddSend(clientAddress, bytesReceived);
        }
    }
}