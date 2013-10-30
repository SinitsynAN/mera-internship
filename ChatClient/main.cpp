#include <stdio.h>
#include "ChatClient.h"

#define PORT 3425
#define MAX_LENGTH 100

int main(int argc, char** argv)
{
    char ip[] = "127.0.0.1";
    char message[] = {0};
    
    ChatClient cc;
    cc.InitSocket(ip, PORT);
    for (int i = 0; i < 3; i++) {
        fgets(message, MAX_LENGTH, stdin);
        cc.Send(message);
    }
            
    return 0;
}