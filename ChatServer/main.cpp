#include <stdlib.h>
#include "ChatServer.h"

#define SERVER_COMM_FILEPATH "/tmp/s_comm"
#define SV_COMM_FILEPATH "/tmp/sv_comm"
#define PORT 3425

int main(int argc, char** argv)
{
    ChatServer cs;
    cs.InitSSocket(SERVER_COMM_FILEPATH, SV_COMM_FILEPATH);
    cs.InitClientSocket(PORT);
    cs.Start();
    
    return 0;
}

