#ifndef MESSAGEMANAGER_H
#define	MESSAGEMANAGER_H

#include <time.h>

#define QUEUE_SIZE 10
#define RESEND_CHANSES 3
#define RESEND_GAP_SEC 2

class MessageManager
{
private:
    int MSG_LENGTH;
    
    char *messages[QUEUE_SIZE];
    int count;
    int first; //pointer like in a circle buffer
    int current; //pointer like in a circle buffer
    
    int sendTries;
    
    bool isSent;
    bool isConfirmed;
    
    time_t currentTime;
    time_t lastSendTime;
    
public:
    MessageManager(int messageLength);
    ~MessageManager();
    bool IsQueueFull();
    void AddToQueue(char *message);
    int TryToSend(char *result);
    bool TryToConfirm();
};

#endif