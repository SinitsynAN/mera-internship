#include "ChatServer.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

ChatServer::ChatServer()
{
    socketForSupervisor = -1;
    isSocketForSupervisorBinded = false;
    isSocketForSupervisorConnected = false;
    
    socketForClients = -1;
    isSocketForClientsBinded = false;
    
    isStarted = false;
    
    memset(buffer, '\0', BUFFER_SIZE * sizeof(char));
    
    for (int i = 0; i < MAX_CLIENT_COUNT; i++) {
        memset(&(clients[i].address), 0, sizeof(struct sockaddr_in));
        clients[i].nickname = NULL;
        clients[i].lastMessageTime = -1;
    }
    clientCount = 0;
    
    serviceMessages.Fill();
    
    logger = new Logger(LOG_FILEPATH);
}

ChatServer::~ChatServer()
{
    close(socketForSupervisor);
    close(socketForClients);
    
    for (int i = 0; i < MAX_CLIENT_COUNT; i++)
        if (strlen(clients[i].nickname) != NULL)
            delete clients[i].nickname;
    
    delete logger;
}

void ChatServer::InitSocketForSupervisor(char *serverCommFilepath, char *supervisorCommFilepath)
{
    if (!isStarted) {
        socketForSupervisor = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (socketForSupervisor < 0)
            logger->Log("socketForSupervisor hasn't been created");
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
    if (bind(socketForSupervisor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        logger->Log("socketForSupervisor hasn't been binded");
    isSocketForSupervisorBinded = true; 
}

void ChatServer::ConnectSocketForSupervisor()
{
    if (connect(socketForSupervisor, (struct sockaddr *) &supervisorAddress, sizeof(supervisorAddress)) < 0)
        logger->Log("socketForSupervisor hasn't been connected");
    isSocketForSupervisorConnected = true;
}

void ChatServer::InitSocketForClients(char *filepath)
{
    if (!isStarted) {
        FILE *config = fopen(filepath, "r");
        if (config == NULL) {
            logger->Log("Can't open configuration file");
            return;
        }

        char port[10] = {0};
        fscanf(config, "%s", port);
        fclose(config);
        
        socketForClients = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketForClients < 0)
            logger->Log("socketForClients hasn't been created");
        fcntl(socketForClients, F_SETFL, O_NONBLOCK);

        clientSocketAddress.sin_family = AF_INET;
        clientSocketAddress.sin_port = htons(atoi(port));
        clientSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind(socketForClients, (struct sockaddr *) &clientSocketAddress, sizeof(clientSocketAddress)) < 0) {
            logger->Log("socketForClients hasn't been binded");
            return;
        }
        
        isSocketForClientsBinded = true;
    }
}

void ChatServer::Start()
{
    if (!isStarted) {
        if (isSocketForSupervisorBinded && isSocketForSupervisorConnected && isSocketForClientsBinded) {
            printf("\nServer has been successfully started\n");
            logger->Log("Server has been successfully started");
            send(socketForSupervisor, serviceMessages.out[0], strlen(serviceMessages.out[0]) + 1, 0);
            Work();
        }
        else
            logger->Log("Problems with the connections");
    }
    else
        logger->Log("Server has been already started");
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

int ChatServer::FindClient(struct sockaddr_in address)
{
    if (clientCount == 0)
        return -1;
    
    for (int i = 0; i < clientCount; i++)
        if (CompareAddresses(address, clients[i].address))
            return i;
}

void ChatServer::RemoveClient(int number)
{
    if (clientCount == 0)
        return;
    
    clients[number].address = clients[clientCount - 1].address;
    clients[number].lastMessageTime = clients[clientCount - 1].lastMessageTime;
    delete clients[number].nickname;
    clients[number].nickname = new char(strlen(clients[clientCount].nickname) + 1);
    strcpy(clients[number].nickname, clients[clientCount - 1].nickname);
    
    memset(&(clients[clientCount - 1].address), 0, sizeof(struct sockaddr_in));
    delete clients[clientCount - 1].nickname;
    clients[clientCount - 1].lastMessageTime = -1;
    
    clientCount--;
}

void ChatServer::AddSend(struct sockaddr_in clientAddress, int messageLength)
{
    int clientNumber = FindClient(clientAddress);
    if (clientNumber < 0) {
        if (clientCount < MAX_CLIENT_COUNT) {
            clients[clientCount].address = clientAddress;
            clients[clientCount].nickname = new char(messageLength);
            strncpy(clients[clientCount].nickname, buffer, messageLength);
            clients[clientCount].lastMessageTime = time(0);
            
            sendto(socketForClients, serviceMessages.out[2], strlen(serviceMessages.out[2]) + 1, 0, 
                    (struct sockaddr *) &(clients[clientCount].address), sizeof(clients[clientCount].address));
            
            clientCount++;
        }
        else
            logger->Log("Can't add a client");
    }
    else {
        sendto(socketForClients, serviceMessages.out[2], strlen(serviceMessages.out[2]) + 1, 0, 
                (struct sockaddr *) &(clients[clientNumber].address), sizeof(clients[clientNumber].address));
        
        clients[clientNumber].lastMessageTime = time(0);
        
        char *nick = clients[clientNumber].nickname;
        for (int i = 0; i < clientCount; i++) {
            sendto(socketForClients, nick, strlen(nick) + 1, 0, 
                    (struct sockaddr *) &(clients[i].address), sizeof(clients[i].address));
            sendto(socketForClients, buffer, messageLength, 0, 
                    (struct sockaddr *) &(clients[i].address), sizeof(clients[i].address));
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
            logger->Log("Select error");
            break;
        }
        else if (selectResult == 0) {
            // timeout is expired
            continue;
        }

        if (FD_ISSET(socketForSupervisor, &readSet)) {
            bytesReceived = recv(socketForSupervisor, buffer, BUFFER_SIZE, 0);
            buffer[bytesReceived] = '\0';
            
            if (strcmp(buffer, serviceMessages.in[0]) == 0)
                send(socketForSupervisor, serviceMessages.out[1], strlen(serviceMessages.out[1]) + 1, 0);
        }

        if (FD_ISSET(socketForClients, &readSet)) {
            struct sockaddr_in clientAddress;
            socklen_t len = sizeof(clientAddress);
            
            bytesReceived = recvfrom(socketForClients, buffer, BUFFER_SIZE, 0, 
                    (struct sockaddr *) &clientAddress, &len);
            buffer[bytesReceived] = '\0';
            
            //printf("%s\n", buffer);
            
            if (strcmp(buffer, serviceMessages.in[2]) == 0) {
                //printf("\nclient-exit\n");
                int n = FindClient(clientAddress);
                //printf("%d\n", n);
                if (n >= 0)
                    RemoveClient(n);
                //printf("%d\n", clientCount);
            }
            
            AddSend(clientAddress, bytesReceived);
        }
    }
}