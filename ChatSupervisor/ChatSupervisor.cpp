#include "ChatSupervisor.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

ChatSupervisor::ChatSupervisor()
{
    socketForServer = -1;
    memset(&supervisorAddress, 0, sizeof(struct sockaddr_un));
    memset(&serverAddress, 0, sizeof(struct sockaddr_un));  
    isSocketForServerBinded = false;
    isSocketForServerConnected = false;
    
    isStarted = false;
    
    serverInfo.Init();
    serverInfo.possibleStartDelay = POSSIBLE_START_DELAY_SEC;
    serverInfo.requestTimeout = REQUEST_TIMEOUT_SEC;
    serverInfo.possibleAnswerDelay = POSSIBLE_ANSWER_DELAY_SEC;
    
    connectionTriesCount = CONNECTION_TRIES_COUNT;
    
    memset(buffer, '\0', BUFFER_SIZE);
    bytesReceived = 0;
    
    serviceMessages.Fill();
    
    logger = new Logger(LOG_FILEPATH);
}

ChatSupervisor::~ChatSupervisor()
{
    close(socketForServer);
    delete logger;
}

void ChatSupervisor::InitSocketForServer(char *supervisorCommFilepath, char *serverCommFilepath)
{
    if (!isStarted) {
        socketForServer = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (socketForServer < 0) {
            logger->Log("socketForServer hasn't been created");
            exit(1);
        }
        fcntl(socketForServer, F_SETFL, O_NONBLOCK);
        
        supervisorAddress.sun_family = AF_UNIX;
        strcpy(supervisorAddress.sun_path, supervisorCommFilepath);
        unlink(supervisorAddress.sun_path);
        
        BindSocketForServer();
        
        serverAddress.sun_family = AF_UNIX;
        strcpy(serverAddress.sun_path, serverCommFilepath);
    }
    else
        printf("\nCan't initialize socket. Supervisor has been already started.\n");
}

void ChatSupervisor::BindSocketForServer()
{
    if (bind(socketForServer, (struct sockaddr *) &supervisorAddress, sizeof(supervisorAddress)) < 0) {
       logger->Log("socketForServer hasn't been binded");
       exit(2);
    }
    isSocketForServerBinded = true; 
}

void ChatSupervisor::ConnectSocketForServer()
{
    if (connect(socketForServer, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        logger->Log("socketForServer hasn't been connected");
        exit(3);
    }
    isSocketForServerConnected = true;
}

void ChatSupervisor::Start()
{
    if (!isStarted) {
        if (isSocketForServerBinded) {
            printf("\nSupervisor has been successfully started.\n");
            logger->Log("Supervisor has been successfully started");
            Work();
        }
        else
            logger->Log("socketForServer hasn't been binded");
    }
    else
        logger->Log("Supervisor has been already started");
}

void ChatSupervisor::Work()
{
    StartServer();
    
    while(1) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(socketForServer, &readSet);

        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(socketForServer, &writeSet);

        timeval timeout;
        timeout.tv_sec = SELECT_TIMEOUT_SEC;
        timeout.tv_usec = 0;

        int maxFd = socketForServer;
        int selectResult = select(maxFd + 1, &readSet, &writeSet, NULL, &timeout);
        if (selectResult < 0) {
            logger->Log("select error");
            break;
        }
        else if (selectResult == 0) {
            //timeout has been expired
            continue;
        }

        if (FD_ISSET(socketForServer, &readSet)) {
            bytesReceived = recv(socketForServer, buffer, BUFFER_SIZE, 0);
            buffer[bytesReceived] = '\0';
            
            OnServerAnswer();
        }
        
        if (!serverInfo.isStarted)
            CheckServerReadiness();
        
        if (FD_ISSET(socketForServer, &writeSet))
            CheckServerAvailability();
    }
}

void ChatSupervisor::StartServer()
{
    pid_t pid = fork();
    if (pid < 0) {
        logger->Log("fork error");
        exit(4);
    }
    else if (pid == 0) {
        execl(SERVER_EXEC_PATH, NULL);
        logger->Log("exec error");
        exit(5);
    }
    else
        serverInfo.serverPID = pid;
}

void ChatSupervisor::RestartServer()
{
    kill(serverInfo.serverPID, SIGKILL);
    waitpid(serverInfo.serverPID, NULL, NULL);
    
    logger->Log("Trying to restart server");
    
    serverInfo.Init();
    serverInfo.possibleStartDelay = POSSIBLE_START_DELAY_SEC;
    serverInfo.requestTimeout = REQUEST_TIMEOUT_SEC;
    serverInfo.possibleAnswerDelay = POSSIBLE_ANSWER_DELAY_SEC;
    
    StartServer();
}

void ChatSupervisor::CheckServerReadiness()
{
    time_t currentTime;
    time(&currentTime);
    if (!serverInfo.isStarted && currentTime - serverInfo.lastConnectionTryTime > serverInfo.possibleStartDelay) {
        if (connectionTriesCount > 0) {
            serverInfo.lastConnectionTryTime = currentTime;
            connectionTriesCount--;
        }
        else {
            logger->Log("Server hasn't been started");
            exit(6);
        }
    }
}

void ChatSupervisor::CheckServerAvailability()
{
    time_t currentTime;
    if (serverInfo.isStarted) { 
        time(&currentTime);
        
        if (currentTime - serverInfo.lastAnswerTime > serverInfo.possibleAnswerDelay)
            serverInfo.isAvailable = false;
        
        if (serverInfo.isAvailable) {
            if (currentTime - serverInfo.lastRequestTime > serverInfo.requestTimeout) {
                send(socketForServer, serviceMessages.out[0], strlen(serviceMessages.out[0]) + 1, 0);
                serverInfo.lastRequestTime = currentTime;
            }
        }
        else {
            logger->Log("Server isn't available");
            RestartServer();
        }
    }
}

void ChatSupervisor::OnServerAnswer()
{
    if (!serverInfo.isStarted && strcmp(buffer, serviceMessages.in[0]) == 0) {
        logger->Log("Server has been started");
        serverInfo.isStarted = true;
        time(&(serverInfo.startTime));
        serverInfo.isAvailable = true;
        serverInfo.lastRequestTime = serverInfo.startTime;
        serverInfo.lastAnswerTime = serverInfo.startTime;
        ConnectSocketForServer();
    }
    else if (serverInfo.isStarted && strcmp(buffer, serviceMessages.in[1]) == 0)
        time(&(serverInfo.lastAnswerTime));
}