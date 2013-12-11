#ifndef CLIENTINFO_H
#define	CLIENTINFO_H

#include <netinet/in.h>
#include <time.h>

struct ClientInfo
{
    struct sockaddr_in address;
    char *nickname;
    time_t lastMessageTime;
};

#endif