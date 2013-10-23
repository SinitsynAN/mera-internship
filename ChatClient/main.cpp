#include <cstdlib>
#include <stdio.h>
#include "IpAddressConverter.h"
#include "ChatClient.h"

using namespace std;

int main(int argc, char** argv)
{
    IpAddressConverter ipc;
    ipc.TryConvert("127.000.000.001");
    unsigned long ip = ipc.GetAddress();
    
    ChatClient cc;
    cc.InitSocket(ip, 3425);
    for (int i = 0; i < 5; i++)
        cc.Send("Message from client\n");
            
    return 0;
}