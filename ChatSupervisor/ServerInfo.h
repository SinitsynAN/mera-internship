#ifndef SERVERINFO_H
#define	SERVERINFO_H

#include <unistd.h>
#include <time.h>

struct ServerInfo
{
    pid_t serverPID;
    time_t lastConnectionTryTime;
    time_t startTime;
    time_t lastRequestTime;
    time_t lastAnswerTime;
    time_t possibleStartDelay;
    time_t requestDelay;
    time_t possibleAnswerDelay;
    bool isStarted;
    bool isAvailable;
    
    void Init()
    {
        serverPID = -1;
        lastConnectionTryTime = 0;
        startTime = 0;
        lastRequestTime = 0;
        lastAnswerTime = 0;
        possibleStartDelay = 0;
        requestDelay = 0;
        possibleAnswerDelay = 0;
        isStarted = false;
        isAvailable = false;
    }
};

#endif

