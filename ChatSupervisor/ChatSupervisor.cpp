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
    isSocketForServerBinded = false;
    isSocketForServerConnected = false;
    
    isStarted = false;
    
    serverInfo.Init();
    serverInfo.possibleStartDelay = POSSIBLE_START_DELAY_SEC;
    serverInfo.requestTimeout = REQUEST_TIMEOUT_SEC;
    serverInfo.possibleAnswerDelay = POSSIBLE_ANSWER_DELAY_SEC;
            
    connectionTriesCount = CONNECTION_TRIES_COUNT;
    
    for (int i = 0; i < BUFFER_SIZE; i ++)
        buffer[i] = 0;
    
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
            logger->Log("socketForServer isn't created");
            //perror("socketForServer isn't created");
        }
        fcntl(socketForServer, F_SETFL, O_NONBLOCK);
        
        supervisorAddress.sun_family = AF_UNIX;
        strcpy(supervisorAddress.sun_path, supervisorCommFilepath);
        unlink(supervisorAddress.sun_path);
        
        BindSocketForServer();
        
        serverAddress.sun_family = AF_UNIX;
        strcpy(serverAddress.sun_path, serverCommFilepath);
    }
}

void ChatSupervisor::BindSocketForServer()
{
    if (bind(socketForServer, (struct sockaddr *) &supervisorAddress, sizeof(supervisorAddress)) < 0) {
       logger->Log("Problems with socketForServer binding");
        //perror("Problems with socketForServer binding");
    }
    isSocketForServerBinded = true; 
}

void ChatSupervisor::ConnectSocketForServer()
{
    if (connect(socketForServer, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        logger->Log("Problems with socketForServer connecting");
        //perror("Problems with socketForServer connecting");
    }
    isSocketForServerConnected = true;
}

void ChatSupervisor::Start()
{
    if (!isStarted) {
        if (isSocketForServerBinded) {
            logger->Log("Supervisor is successfully started");
            printf("\nSupervisor is successfully started\n");
            Work();
        }
        else {
            logger->Log("socketForServer isn't binded");
            //perror("socketForServer isn't binded");
        }
    }
    else {
        logger->Log("Server has been already started");
        //perror("Server has been already started");
    }
}

void ChatSupervisor::Work()
{
    StartServer();
    int bytesReceived = 0;
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
            logger->Log("Problems with select");
            //perror("Problems with select");
            break;
        }
        else if (selectResult == 0) {
            // timeout is expired
            continue;
        }

        if (FD_ISSET(socketForServer, &readSet)) {
            bytesReceived = recv(socketForServer, buffer, BUFFER_SIZE, 0);
            buffer[bytesReceived] = '\0';
            printf("%s\n", buffer);
            fflush(stdout);
            
            OnServerAnswer();
        }
        
        if (!serverInfo.isStarted)
                CheckServerReadiness();
        
        if (FD_ISSET(socketForServer, &writeSet)) {
            CheckServerAvailability();
        }
    }
}

void ChatSupervisor::StartServer()
{
    pid_t pid = fork();
    if (pid < 0)
        logger->Log("Fork error");
        //perror("Fork error");
    else if (pid == 0) {
        execl("chatserver", NULL);
        logger->Log("Exec error");
        //perror("Exec error");
    }
    else
        serverInfo.serverPID = pid;
}

void ChatSupervisor::RestartServer()
{
    kill(serverInfo.serverPID, SIGKILL);
    waitpid(serverInfo.serverPID, NULL, NULL);
    
    logger->Log("Trying to restart server");
    printf("\nTrying to restart server...\n");
    
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
            logger->Log("Server hasn't started");
            //perror("Server hasn't started");
            exit(1); // ~!~!~!~!~!~
        }
    }
}

void ChatSupervisor::CheckServerAvailability()
{
    time_t currentTime;
    if (serverInfo.isStarted) {
        
        time(&currentTime);
        
        if (currentTime - serverInfo.lastAnswerTime > serverInfo.possibleAnswerDelay) {
            serverInfo.isAvailable = false;
        }
        
        if (serverInfo.isAvailable) {
            if (currentTime - serverInfo.lastRequestTime > serverInfo.requestTimeout) {
                send(socketForServer, "Are you alive?", 15, 0);
                serverInfo.lastRequestTime = currentTime;
            }
        }
        else
            RestartServer();
    }
}

void ChatSupervisor::OnServerAnswer()
{
    if (!serverInfo.isStarted && strcmp(buffer, "I've started") == 0) {
        serverInfo.isStarted = true;
        time(&(serverInfo.startTime));
        serverInfo.isAvailable = true;
        serverInfo.lastRequestTime = serverInfo.startTime;
        serverInfo.lastAnswerTime = serverInfo.startTime;
        ConnectSocketForServer();
    }
    else if (strcmp(buffer, "I'm alive") == 0) {
        time(&(serverInfo.lastAnswerTime));
    }
}