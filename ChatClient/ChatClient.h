#ifndef CHATCLIENT_H
#define	CHATCLIENT_H

#include <gtk/gtk.h>
#include <netinet/in.h>

#include <MessageManager.h>

#define BUFFER_SIZE 1024
#define SELECT_TIMEOUT_SEC 10

class ChatClient
{
private:
    int socketForServer;
    struct sockaddr_in clientAddress;
    struct sockaddr_in serverAddress;
    bool isSocketForServerOK;
    char *clientNickname;
    
    char buffer[BUFFER_SIZE];
    
    MessageManager *messageManager;
    int confirmationCount;
    
    GtkWidget *window;
    GtkWidget *table;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *textView;
    GtkWidget *scrolledWindow;
    
    void Work();
    
    void OnSendButtonClick();
    static void SendButtonClickProxy(GtkWidget *widget, gpointer data);
    void OnCrossClick();
    static void CrossClickProxy(GtkWidget *widget, gpointer data);
    
    void ShowIncomingMessage(char *message);
    
public:
    ChatClient();
    ~ChatClient();
    void SetNickname(char *nickname);
    void InitSocketForServer(char *filepath);
    void InitGraphics(int argc, char *argv[]);
    void Start();
    void Send(char *message);
};

#endif