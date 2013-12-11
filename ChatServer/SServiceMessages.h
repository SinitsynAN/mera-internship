#ifndef SSERVICEMESSAGES_H
#define	SSERVICEMESSAGES_H

#include <string.h>

#define MAX_LENGTH 30

struct SServiceMessages
{
    char out[4][MAX_LENGTH];
    char in[3][MAX_LENGTH];
    
    void Fill()
    {       
        strcpy(out[0], "I've started");
        strcpy(out[1], "I'm alive");
        strcpy(out[2], "MSG_RCVD");
        strcpy(out[3], "CHCK_MSG");
        
        strcpy(in[0], "Are you alive?");
        strcpy(in[1], "CNFRM_MSG");
        strcpy(in[2], "XT_MSG");
    }
};

#endif