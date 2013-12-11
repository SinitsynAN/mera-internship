#ifndef SVSVSERVICEMESSAGES_H

#include <string.h>

#define MAX_LENGT 30

struct SVServiceMessages
{
    char out[1][MAX_LENGT];
    char in[2][MAX_LENGT];
    
    void Fill()
    {
        strcpy(out[0], "Are you alive?");
        
        strcpy(in[0], "I've started");
        strcpy(in[1], "I'm alive");
    }
    
};

#endif