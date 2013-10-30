#include <stdlib.h>
#include "ChatServer.h"

#define PORT 3425

int main(int argc, char** argv)
{
    ChatServer cs;
    cs.InitSocket(PORT);
    cs.Start();
    
    return 0;
}

