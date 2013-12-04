#include <stdio.h>
#include <string.h>

#include "ChatClient.h"

#define MAX_NICKNAME_LEGNTH 15
#define MAX_MESSAGE_LENGTH 100

int main(int argc, char** argv)
{
    char nickname[MAX_NICKNAME_LEGNTH] = {0};
    
    printf("Enter your nickname:\n");
    fgets(nickname, MAX_NICKNAME_LEGNTH, stdin);
    fflush(stdin);
    
    ChatClient cc;
    cc.SetNickname(nickname);
    cc.InitSocketForServer("client_config.txt");
    cc.InitGraphics(argc, argv);
    cc.Start();
            
    return 0;
}