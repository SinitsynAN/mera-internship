#include "ChatClient.h"

#include <stdio.h>
#include <string.h>

#define MAX_SIZE 1024
#define MAX_NICKNAME_LEGNTH 16

int main(int argc, char** argv)
{
    char buffer[MAX_SIZE] = {0};
    char nickname[MAX_NICKNAME_LEGNTH] = {0};
    
    bool isOK = false;
    
    do {
        printf("Enter your nickname:\n");
        fgets(buffer, MAX_SIZE, stdin);
        fflush(stdin);
        
        int len = strlen(buffer);
        if (buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
        
        if (len <= MAX_NICKNAME_LEGNTH) {
            strncpy(nickname, buffer, len);
            isOK = true;
        }
        else {
            memset(buffer, '\0', MAX_SIZE);
            printf("It's longer then %d symbols.\n", MAX_NICKNAME_LEGNTH - 1);
        }
    }
    while (!isOK);
    
    ChatClient cc;
    cc.SetNickname(nickname);
    cc.InitGraphics(argc, argv);
    cc.Start();
            
    return 0;
}