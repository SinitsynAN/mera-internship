#ifndef CHATLOGGER_H
#define	CHATLOGGER_H

#include <stdio.h>

#define FILEPATH_LENGTH 100;
#define TIME_STR_SIZE 80

class ChatLogger
{
private:
    FILE *log;
    char *logFilepath;
    bool logExists;
    bool isLogOpened;
    
    char currentTime[TIME_STR_SIZE];
    
    void CreateLog();
    
    void GetTime();
    
public:
    ChatLogger(char *filepath);
    ~ChatLogger();
    void Open();
    void Close();
    void Log(char *message);
};

#endif