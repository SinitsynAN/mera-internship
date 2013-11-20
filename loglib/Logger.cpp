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
    //isLogOpened = false;
    
    for (int i = 0; i < TIME_STR_SIZE; i++)
        currentTimeStr[i] = 0;
}

Logger::~Logger()
{
    //Close();
    delete [] logFilepath;
}

void Logger::Create()
{
    log = fopen(logFilepath, "w");
    if (log == NULL) {
        perror("Log isn't created");
        doesLogExist = false;
    }
    else {
        fclose(log);
        doesLogExist = true;
    }
}

/*void Logger::Open()
{
    if (doesLogExist) {
        if (!isLogOpened) {
            log = fopen(logFilepath, "a");
            isLogOpened = true;
        }
    }
    else {
        perror("Log doesn't exist");
    }
}

void Logger::Close()
{
    if (isLogOpened) {
        fclose(log);
        isLogOpened = false;
    }
}

void Logger::Log(char* message)
{
    if (isLogOpened) {
        GetTime();
        fprintf(log, "%s ", currentTimeStr);
        fprintf(log, "%s\n", message);
    }
    else {
        perror("Log isn't opened");
    }
}*/

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
    sprintf(currentTimeStr, "[%d:%d:%d]", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
}