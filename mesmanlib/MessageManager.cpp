#include "MessageManager.h"

#include <string.h>

MessageManager::MessageManager(int messageLength)
{
    MSG_LENGTH = messageLength;
    
    for (int i = 0; i < QUEUE_SIZE; i++) {
        messages[i] = new char[MSG_LENGTH];
        memset(messages[i], '\0', MSG_LENGTH * sizeof(char));
    }
    
    first = 0;
    current = 0;
    count = 0;
    
    sendTries = RESEND_CHANSES;
    
    isSent = false;
    isConfirmed = false;
    
    currentTime = 0;
    lastSendTime = 0;
}

MessageManager::~MessageManager()
{
    for (int i = 0; i < QUEUE_SIZE; i++)
        delete messages[i];
}

bool MessageManager::IsQueueFull()
{
    if (count < QUEUE_SIZE)
        return false;
    else
        return true;
}

void MessageManager::AddToQueue(char *message)
{
    strcpy(messages[current], message);
    count++;
    if (count == 1)
        first = current;
    current = (current + 1) % QUEUE_SIZE;
}

int MessageManager::TryToSend(char *result)
{
    if (count == 0)
        return 0;
    
    if (!isSent && !isConfirmed) {
        strcpy(result, messages[first]);
        time(&lastSendTime);
        isSent = true;
        sendTries--;
        return 1;
    }
    
    if (isSent && !isConfirmed) {
        time(&currentTime);
        if (sendTries > 0 && currentTime - lastSendTime > RESEND_GAP_SEC) {
            strcpy(result, messages[first]);
            time(&lastSendTime);
            sendTries--;
            return 1;
        }
        if (sendTries == 0)
            return 2;
    }
    
    if (isSent && isConfirmed) {
        first = (first + 1) % QUEUE_SIZE;
        count--;
        isSent = false;
        isConfirmed = false;
        sendTries = RESEND_CHANSES;
        return 3;
    }
    
    return 4;
}

bool MessageManager::TryToConfirm()
{
    if (isSent) {
        isConfirmed = true;
        return true;
    }
    return false;
}