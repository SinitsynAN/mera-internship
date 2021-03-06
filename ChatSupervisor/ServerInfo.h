#ifndef SERVERINFO_H
#define	SERVERINFO_H

#include <time.h>
#include <unistd.h>

struct ServerInfo
{
    pid_t serverPID;
    time_t lastConnectionTryTime;
    time_t startTime;
    time_t lastRequestTime;
    time_t lastAnswerTime;
    
    time_t possibleStartDelay;
    time_t requestTimeout;
    time_t possibleAnswerDelay;
    
    bool isStarted;
    bool isAvailable;
    
    void Init()
    {
        serverPID = -1;
        lastConnectionTryTime = -1;
        startTime = -1;
        lastRequestTime = -1;
        lastAnswerTime = -1;
        
        isStarted = false;
        isAvailable = false;
    }
};

#endif

