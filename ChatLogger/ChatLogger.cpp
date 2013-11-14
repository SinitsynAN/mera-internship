#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ChatLogger.h"

ChatLogger::ChatLogger(char* filepath)
{
    int len = strlen(filepath);
    logFilepath = new char[len + 1];
    strcpy(logFilepath, filepath);
    
    CreateLog();
    isLogOpened = false;
    
    for (int i = 0; i < TIME_STR_SIZE; i++)
        currentTime[i] = 0;
}

ChatLogger::~ChatLogger()
{
    Close();
    delete [] logFilepath;
}

void ChatLogger::CreateLog()
{
    log = fopen(logFilepath, "w");
    if (log == NULL) {
        logExists = false;
        perror("Log isn't created");
    }
    else {
        logExists = true;
        fclose(log);
    }
}

void ChatLogger::Open()
{
    if (logExists && !isLogOpened) {
        log = fopen(logFilepath, "a");
        isLogOpened = true;
    }
}

void ChatLogger::Close()
{
    if (isLogOpened) {
        fclose(log);
        isLogOpened = false;
    }
}

void ChatLogger::Log(char* message)
{
    if (isLogOpened) {
        GetTime();
        fprintf(log, "%s ", currentTime);
        fprintf(log, "%s\n", message);
    }
}

void ChatLogger::GetTime()
{
    time_t t;
    tm *timeInfo;
    
    time(&t);
    timeInfo = localtime(&t);
    sprintf(currentTime, "[%d:%d:%d]", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
}