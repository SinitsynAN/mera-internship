#ifndef CONFIG_H
#define	CONFIG_H

#include <string.h>

struct Config
{
    char servAddr[17];
    char servPort[6];
    char cliPort[6];
    
    void Init()
    {
        memset(servAddr, '\0', 17);
        memset(servPort, '\0', 6);
        memset(cliPort, '\0', 6);
    }
};

#endif