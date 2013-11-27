#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "ChatClient.h"

ChatClient::ChatClient()
{
    socketForServer = -1;
    isSocketForServerOK = false;
    for (int i = 0; i < BUFFER_SIZE; i++)
        buffer[i] = 0;
    
    clientNickname = new char[7];
    strcpy(clientNickname, "Anonym");
}

ChatClient::~ChatClient()
{
    close(socketForServer);
    delete [] clientNickname;
}

void ChatClient::SetNickname(char *nickname)
{
    delete [] clientNickname;
    
    int len = strlen(nickname);
    clientNickname = new char[len + 1];
    strcpy(clientNickname, nickname);
    if (clientNickname[len - 1] == '\n')
        clientNickname[len - 1] = '\0';
}

void ChatClient::InitSocketForServer(char *targetAddress, unsigned short targetPort)
{
    if (!isSocketForServerOK) {
        socketForServer = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketForServer < 0) {
            perror("socketForServer isn't created");
            return;
        }
        fcntl(socketForServer, F_SETFL, O_NONBLOCK);

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(targetPort); 
        if (inet_pton(AF_INET, targetAddress, &serverAddress.sin_addr) <= 0) {
            perror("Invalid IP-address");
            return ;
        }
        
        clientAddress.sin_family = AF_INET;
        clientAddress.sin_port = htons(3426);
        clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind(socketForServer, (struct sockaddr *) &clientAddress, sizeof(clientAddress)) < 0) {
            perror("Problems with socketForServer binding");
            return;
        }
        
        isSocketForServerOK = true;
    }
    else {
        perror("socketForServer has been already opened");
    }
}

void ChatClient::InitGraphics(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    
    // window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "ChatClient"); 
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(gtk_main_quit), NULL); 
    //
    
    // table
    table = gtk_table_new(3, 3, TRUE);
    gtk_container_add(GTK_CONTAINER(window), table);
    //
    
    // button
    button = gtk_button_new_with_label("Send");
    gtk_table_attach_defaults(GTK_TABLE(table), button, 2, 3, 2, 3);
    gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(SendButtonClickProxy), this);
    //
    
    // entry
    entry = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 0, 2, 2, 3);
    //
    
    // textView
    textView = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textView), FALSE);
    //
    
    // scrolledWindow
    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), textView);
    gtk_table_attach_defaults(GTK_TABLE(table), scrolledWindow, 0, 3, 0, 2);
    //

    gtk_widget_show_all(window);
    
    while (gtk_events_pending())
        gtk_main_iteration();
}

void ChatClient::Start()
{
    if (isSocketForServerOK) {
        printf("ChatClient has been successfully stared\n");
        Work();
    }
    else {
        perror("Socket hasn't been opened");
    }
}

void ChatClient::Work()
{
    int bytesReceived = 0;
    while(1) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(socketForServer, &readSet);

        timeval timeout;
        timeout.tv_sec = 0;
        //timeout.tv_sec = SELECT_TIMEOUT_SEC;
        timeout.tv_usec = 0;

        while (gtk_events_pending())
            gtk_main_iteration();
        
        int maxFd = socketForServer;
        int selectResult = select(maxFd + 1, &readSet, NULL, NULL, &timeout);
        if (selectResult < 0) {
            perror("Problems with select");
            break;
        }
        else if (selectResult == 0) {
            // timeout is expired
            continue;
        }

        if (FD_ISSET(socketForServer, &readSet)) {
            bytesReceived = recvfrom(socketForServer, buffer, BUFFER_SIZE, 0, NULL, NULL);
            buffer[bytesReceived] = '\0';
            printf(">[C]: %s\n", buffer);
            ShowIncomingMessage(buffer);
        }
    }
}

void ChatClient::ShowIncomingMessage(char* message)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    const gchar *messageToAdd;
    
    messageToAdd = (gchar *) message;
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, messageToAdd, -1);
}

void ChatClient::Send(char *message)
{
    int msgSize = strlen(message) + 1;
    if (isSocketForServerOK && msgSize > 1) {
        sendto(socketForServer, clientNickname, strlen(clientNickname) + 1, 0, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
        sendto(socketForServer, message, msgSize, 0, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    }
}

void ChatClient::OnSendButtonClick()
{
    const gchar *entryText;   
    entryText = gtk_entry_get_text(GTK_ENTRY(entry));
    
    Send((char *)entryText);
}

void ChatClient::SendButtonClickProxy(GtkWidget* widget, gpointer data)
{
    ChatClient *cc = (ChatClient *)data;
    cc->OnSendButtonClick();
}