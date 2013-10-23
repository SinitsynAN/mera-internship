#include <cstdlib>
#include "ChatServer.h"

using namespace std;

int main(int argc, char** argv)
{
    ChatServer cs;
    cs.InitSocket(3425);
    cs.Start();
    
    return 0;
}

