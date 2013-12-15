#ifndef CLIENTINFO_H
#define	CLIENTINFO_H

#include <netinet/in.h>
#include <string.h>

struct ClientInfo
{
    struct sockaddr_in address;
    char *nickname;
    bool isConfirmed;
    
    void Init()
    {
        memset(&address, 0, sizeof(struct sockaddr_in));
        nickname = NULL;
        isConfirmed = false;
    }
};

#endif