#include <stdio.h>
#include <string.h>
#include "ChatClient.h"

#define PORT 3425
#define MAX_LENGTH 100

int main(int argc, char** argv)
{
    char ip[] = "127.0.0.1";
    char message[MAX_LENGTH] = {0};
    
    ChatClient cc;
    cc.InitSocket(ip, PORT);
    while(1) {
        fgets(message, MAX_LENGTH, stdin);
        fflush(stdin);
        if (strcmp(message, "quit\n") == 0)
            break;
        cc.Send(message);
    }
            
    return 0;
}