#include "ChatSupervisor.h"

#define SV_COMM_FILEPATH "/tmp/sv_comm"
#define SERVER_COMM_FILEPATH "/tmp/s_comm"

int main(int argc, char** argv)
{
    ChatSupervisor csv;
    csv.InitSocketForServer(SV_COMM_FILEPATH, SERVER_COMM_FILEPATH);
    csv.Start();
    
    return 0;
}

