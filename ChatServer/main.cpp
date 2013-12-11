#include "ChatServer.h"

#define SERVER_COMM_FILEPATH "/tmp/s_comm"
#define SV_COMM_FILEPATH "/tmp/sv_comm"

int main(int argc, char** argv)
{
    ChatServer cs;
    cs.InitSocketForSupervisor(SERVER_COMM_FILEPATH, SV_COMM_FILEPATH);
    cs.InitSocketForClients("server_config");
    cs.Start();
    
    return 0;
}

