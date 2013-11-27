#include <stdio.h>
#include <string.h>
#include "ChatClient.h"

#define PORT 3425
#define MAX_NICKNAME_LEGNTH 15
#define MAX_MESSAGE_LENGTH 100

int main(int argc, char** argv)
{
    char ip[] = "127.0.0.1";
    char nickname[MAX_NICKNAME_LEGNTH] = {0};
    char message[MAX_MESSAGE_LENGTH] = {0};
    
    printf("Enter your nickname:\n");
    fgets(nickname, MAX_NICKNAME_LEGNTH, stdin);
    fflush(stdin);
    
    ChatClient cc;
    cc.SetNickname(nickname);
    cc.InitSocketForServer(ip, PORT);
    /*while(1) {
        printf(">: ");
        fgets(message, MAX_MESSAGE_LENGTH, stdin);
        fflush(stdin);
        if (strcmp(message, "quit\n") == 0)
            break;
        cc.Send(message);
    }*/
    cc.InitGraphics(argc, argv);
    cc.Start();
            
    return 0;
}