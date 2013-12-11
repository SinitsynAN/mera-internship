#ifndef CSERVICEMESSAGES_H
#define	CSERVICEMESSAGES_H

#include <string.h>

#define MAX_LENGTH 30

struct CServiceMessages
{
    char out[2][MAX_LENGTH];
    char in[2][MAX_LENGTH];
    
    void Fill()
    {
        strcpy(out[0], "CNFRM_MSG");
        strcpy(out[1], "XT_MSG");
        
        strcpy(in[0], "MSG_RCVD");
        strcpy(in[1], "CHCK_MSG");
    }
};

#endif