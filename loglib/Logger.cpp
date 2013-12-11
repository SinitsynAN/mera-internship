#include "Logger.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

Logger::Logger(char* filepath)
{
    int len = strlen(filepath);
    logFilepath = new char[len + 1];
    strcpy(logFilepath, filepath);
    
    Create();
    
    memset(currentTimeStr, '\0', TIME_STR_SIZE);
}

Logger::~Logger()
{
    delete [] logFilepath;
}

void Logger::Create()
{
    log = fopen(logFilepath, "w");
    if (log == NULL) {
        perror("Log hasn't been created");
        doesLogExist = false;
    }
    else {
        fclose(log);
        doesLogExist = true;
    }
}

void Logger::Log(char* message)
{
    if (doesLogExist) {
        GetTime();
        log = fopen(logFilepath, "a");
        fprintf(log, "%s ", currentTimeStr);
        fprintf(log, "%s\n", message);
        fclose(log);
    }
    else {
        perror("Log doesn't exist");
    }
}

void Logger::GetTime()
{
    time_t currentTime;
    tm *timeInfo;
    
    time(&currentTime);
    timeInfo = localtime(&currentTime);
    memset(currentTimeStr, '\0', TIME_STR_SIZE);
    sprintf(currentTimeStr, "[%02d:%02d:%02d]", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
}