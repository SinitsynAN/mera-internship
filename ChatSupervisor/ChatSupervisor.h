#ifndef CHATSUPERVISOR_H
#define	CHATSUPERVISOR_H

#include "ServerInfo.h"
#include "SVServiceMessages.h"

#include <Logger.h>

#include <sys/un.h>

#define POSSIBLE_START_DELAY_SEC 5
#define REQUEST_TIMEOUT_SEC 5
#define POSSIBLE_ANSWER_DELAY_SEC 10

#define CONNECTION_TRIES_COUNT 5

#define SELECT_TIMEOUT_SEC 5

#define BUFFER_SIZE 1024

#define LOG_FILEPATH "/tmp/sv_log"

#define SERVER_EXEC_PATH "chatserver"

class ChatSupervisor
{
private:
    int socketForServer;
    struct sockaddr_un supervisorAddress;
    struct sockaddr_un serverAddress;
    bool isSocketForServerBinded;
    bool isSocketForServerConnected;
    
    bool isStarted;
    
    ServerInfo serverInfo;
    int connectionTriesCount;
    
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    
    SVServiceMessages serviceMessages;
    
    Logger *logger;
    
    void BindSocketForServer();
    void ConnectSocketForServer();
    
public:
    ChatSupervisor();
    ~ChatSupervisor();
    
    void InitSocketForServer(char *supervisorCommFilepath, char *serverCommFilepath);
    void Start();
    void Work();
    
    void StartServer();
    void RestartServer();
    void CheckServerReadiness();
    void CheckServerAvailability();
    void OnServerAnswer();
};

#endif