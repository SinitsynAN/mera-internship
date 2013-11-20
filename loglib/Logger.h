#ifndef LOGGER_H
#define	LOGGER_H

#include <stdio.h>

#define FILEPATH_MAX_LENGTH 100;
#define TIME_STR_SIZE 80

class Logger
{
private:
    FILE *log;
    char *logFilepath;
    bool doesLogExist;
    //bool isLogOpened;
    
    char currentTimeStr[TIME_STR_SIZE];
    
    void Create();
    
    void GetTime();
    
public:
    Logger(char *filepath);
    ~Logger();
    /*void Open();
    void Close();*/
    void Log(char *message);
};

#endif

