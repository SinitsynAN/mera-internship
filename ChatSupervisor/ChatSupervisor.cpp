#include "ChatSupervisor.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

ChatSupervisor::ChatSupervisor()
{
    socketForServer = -1;
    isSocketForServerBinded = false;
    isSocketForServerConnected = false;
    
    isStarted = false;
    
    serverInfo.serverPID = -1;
    serverInfo.isStarted = false;
    serverInfo.isAvailable = false;
    serverInfo.initTime = 0;
    serverInfo.lastConnectionTryTime = 0;
    serverInfo.startTime = 0;
    serverInfo.lastRequestTime = 0;
    serverInfo.lastAnswerTime = 0;
    serverInfo.possibleDelay = POSSIBLE_DELAY_SEC;
            
    connectionTriesCount = CONNECTION_TRIES_COUNT;
    
    for (int i = 0; i < BUFFER_SIZE; i ++)
        buffer[i] = 0;
}

ChatSupervisor::~ChatSupervisor()
{
    close(socketForServer);
}

void ChatSupervisor::InitSocketForServer(char supervisorCommFilepath[], char serverCommFilepath[])
{
    if (!isStarted) {
        socketForServer = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (socketForServer < 0) {
            perror("socketForServer isn't created");
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
            perror("Problems with socketForServer binding");
        }
        isSocketForServerBinded = true; 
}

void ChatSupervisor::ConnectSocketForServer()
{
    if (connect(socketForServer, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("Problems with socketForServer connecting");
    }
    isSocketForServerConnected = true;
}

void ChatSupervisor::Start()
{
    if (!isStarted) {
        if (isSocketForServerBinded) {
            printf("Supervisor is successfully started\n");
            Work();
        }
        else {
            perror("socketForServer isn't binded");
        }
    }
    else {
        perror("Server has been already started");
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
            perror("Problems with select");
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
        perror("Fork error");
    else if (pid == 0) {
        execv("/home/Silgur/chatserver", NULL); // ~!~!~!~!~!~!~!~!~!~!~!~!~
        perror("Exec error");
    }
    else
        serverInfo.serverPID = pid;
}

void ChatSupervisor::RestartServer()
{
    kill(serverInfo.serverPID, SIGKILL);
    waitpid(serverInfo.serverPID, NULL, NULL);
    
    serverInfo.serverPID = -1;
    serverInfo.isStarted = false;
    serverInfo.isAvailable = false;
    serverInfo.initTime = 0;
    serverInfo.lastConnectionTryTime = 0;
    serverInfo.startTime = 0;
    serverInfo.lastRequestTime = 0;
    serverInfo.lastAnswerTime = 0;
    
    StartServer();
}

void ChatSupervisor::CheckServerReadiness()
{
    time_t currentTime;
    time(&currentTime);
    if (!serverInfo.isStarted && currentTime - serverInfo.lastConnectionTryTime > serverInfo.possibleDelay) {
        if (connectionTriesCount > 0) {
            serverInfo.lastConnectionTryTime = currentTime;
            connectionTriesCount--;
        }
        else {
            printf("Server hasn't started\n");
        }
    }
}

void ChatSupervisor::CheckServerAvailability()
{
    time_t currentTime;
    if (serverInfo.isStarted) {
        
        time(&currentTime);
        
        if (currentTime - serverInfo.lastAnswerTime > 2 * serverInfo.possibleDelay) {
            serverInfo.isAvailable = false;
        }
        
        if (serverInfo.isAvailable) {
            if (currentTime - serverInfo.lastRequestTime > serverInfo.possibleDelay) {
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