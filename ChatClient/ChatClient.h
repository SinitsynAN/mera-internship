#ifndef CHATCLIENT_H
#define	CHATCLIENT_H

#include "Config.h"
#include "CServiceMessages.h"

#include <Logger.h>
#include <MessageManager.h>

#include <gtk/gtk.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

#define LOG_PATH "/tmp/c_log"

class ChatClient
{
private:
    int socketForServer;
    struct sockaddr_in clientAddress;
    struct sockaddr_in serverAddress;
    bool isSocketForServerCreated;
    bool isSocketForServerBinded;
    bool isServerAddressCorrect;
    
    Config config;
    
    char *clientNickname;
    
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    
    MessageManager *messageManager;
    int confirmationCount;
    
    CServiceMessages serviceMessages;
    
    Logger *logger;
    
    bool IsSocketForServerValid();
    
    void InitSocketForServer();
    void Send(char *message);
    void Work();
    
    
    GtkWidget *window;
    GtkWidget *table;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *textView;
    GtkWidget *scrolledWindow;
    
    void OnSettingsButtonClick();
    static void SettingsButtonClickProxy(GtkWidget *widget, gpointer data);
    void OnSendButtonClick();
    static void SendButtonClickProxy(GtkWidget *widget, gpointer data);
    void OnExitButtonClick();
    static void ExitButtonClickProxy(GtkWidget *widget, gpointer data);
    void OnCrossClick();
    static void CrossClickProxy(GtkWidget *widget, gpointer data);
    
    void ShowIncomingMessage(char *message);
    
public:
    ChatClient();
    ~ChatClient();
    
    void SetNickname(char *nickname);
    void InitGraphics(int argc, char *argv[]);
    void Start();
};

#endif