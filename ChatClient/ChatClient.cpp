#include "ChatClient.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

ChatClient::ChatClient()
{
    socketForServer = -1;
    isSocketForServerOK = false;
    
    memset(buffer, 0, BUFFER_SIZE * sizeof(char));
    
    messageManager = new MessageManager(BUFFER_SIZE);
    confirmationCount = 0;
    
    serviceMessages.Fill();
    
    logger = new Logger(LOG_PATH);
    
    clientNickname = new char[7];
    strcpy(clientNickname, "Anonym");
}

ChatClient::~ChatClient()
{
    close(socketForServer);
    delete messageManager;
    delete logger;
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

void ChatClient::InitSocketForServer(char* filepath)
{
    FILE *config = fopen(filepath, "r");
    if (config == NULL) {
        logger->Log("Can't open configuration file");
        return;
    }
    
    char servAddress[16] = {0};
    char servPort[10] = {0};
    char clientPort[10] = {0};
    fscanf(config, "%s", servAddress);
    fscanf(config, "%s", servPort);
    fscanf(config, "%s", clientPort);
    fclose(config);
    
    if (!isSocketForServerOK) {
        socketForServer = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketForServer < 0) {
            logger->Log("socketForServer hasn't been created");
            return;
        }
        fcntl(socketForServer, F_SETFL, O_NONBLOCK);

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(atoi(servPort)); 
        if (inet_pton(AF_INET, servAddress, &serverAddress.sin_addr) <= 0) {
            logger->Log("Invalid IP-address");
            return ;
        }
        
        clientAddress.sin_family = AF_INET;
        clientAddress.sin_port = htons(atoi(clientPort));
        clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind(socketForServer, (struct sockaddr *) &clientAddress, sizeof(clientAddress)) < 0) {
            logger->Log("socketForServer hasn't been binded");
            return;
        }
        
        isSocketForServerOK = true;
    }
    else {
        logger->Log("socketForServer has been already initialized");
    }
}

void ChatClient::InitGraphics(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "ChatClient"); 
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(CrossClickProxy), NULL); 
    
    table = gtk_table_new(3, 3, TRUE);
    gtk_container_add(GTK_CONTAINER(window), table);
    
    button = gtk_button_new_with_label("Send");
    gtk_table_attach(GTK_TABLE(table), button, 2, 3, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
    gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(SendButtonClickProxy), this);
    
    entry = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 0, 2, 2, 3);
    gtk_signal_connect(GTK_OBJECT(entry), "activate", GTK_SIGNAL_FUNC(SendButtonClickProxy), this);
    
    textView = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textView), FALSE);
    
    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), textView);
    gtk_table_attach_defaults(GTK_TABLE(table), scrolledWindow, 0, 3, 0, 2);

    gtk_widget_show_all(window);
}

void ChatClient::Start()
{
    if (isSocketForServerOK) {
        printf("\nClient has been successfully started\n");
        logger->Log("Client has been successfully started");
        Work();
    }
    else {
        logger->Log("socketForServer hasn't been opened");
    }
}

void ChatClient::Work()
{
    messageManager->AddToQueue(clientNickname);
    
    int bytesReceived = 0;
    while(1) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(socketForServer, &readSet);
        
        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(socketForServer, &writeSet);

        timeval timeout;
        timeout.tv_sec = 0;
        //timeout.tv_sec = SELECT_TIMEOUT_SEC;
        timeout.tv_usec = 0;

        while (gtk_events_pending())
            gtk_main_iteration();
        
        int maxFd = socketForServer;
        int selectResult = select(maxFd + 1, &readSet, &writeSet, NULL, &timeout);
        if (selectResult < 0) {
            logger->Log("Select error");
            break;
        }
        else if (selectResult == 0) {
            // timeout is expired
            continue;
        }
        
        if (FD_ISSET(socketForServer, &writeSet)) {
            if (confirmationCount > 0)
                messageManager->TryToConfirm();
            
            char writeBuf[BUFFER_SIZE] = {0};
            int res = messageManager->TryToSend(writeBuf);
            if (res == 1)
                Send(writeBuf);
            else if (res == 2) {
                logger->Log("Can't connect to server");
                exit(2);
            }
            else if (res == 3) {
                confirmationCount--;
            }
        }

        if (FD_ISSET(socketForServer, &readSet)) {
            bytesReceived = recvfrom(socketForServer, buffer, BUFFER_SIZE, 0, NULL, NULL);
            buffer[bytesReceived] = '\0';
            
            if (strcmp(buffer, serviceMessages.in[0]) == 0) {
                confirmationCount++;
            }
            else if (strcmp(buffer, serviceMessages.in[1]) == 0) { // ~!~!~!~!~!~!~!~!~!~!~
                if (!messageManager->IsQueueFull())
                    messageManager->AddToQueue(serviceMessages.out[0]);
            }
            else {
                ShowIncomingMessage(buffer);
            }
        }
    }
}

void ChatClient::ShowIncomingMessage(char* message)
{
    GtkTextBuffer *buffer;
    GtkTextMark *mark;
    GtkTextIter iter;
    
    const gchar *messageToAdd;
    messageToAdd = (gchar *) message;
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
    
    mark = gtk_text_buffer_get_insert(buffer);
    gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark);
    
    if (gtk_text_buffer_get_char_count(buffer) > 0)
        gtk_text_buffer_insert(buffer, &iter, "\n", 1);
    gtk_text_buffer_insert(buffer, &iter, messageToAdd, -1);
}

void ChatClient::Send(char *message)
{
    int msgSize = strlen(message) + 1;
    if (isSocketForServerOK && msgSize > 1) {
        //printf("%s\n", message);
        sendto(socketForServer, message, msgSize, 0, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    }
}

void ChatClient::OnSendButtonClick()
{
    const gchar *entryText;   
    entryText = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (!messageManager->IsQueueFull())
        if (strlen((char *)entryText) > 0)
            messageManager->AddToQueue((char *)entryText);
    else {
        printf("\nMessage queue is full\n");
        logger->Log("Message queue is full");
    }
    //Send(serviceMessages.out[1]);
    
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

void ChatClient::SendButtonClickProxy(GtkWidget* widget, gpointer data)
{
    ChatClient *cc = (ChatClient *)data;
    cc->OnSendButtonClick();
}

void ChatClient::OnCrossClick()
{
    Send(serviceMessages.out[1]);
    exit(0);
}

void ChatClient::CrossClickProxy(GtkWidget* widget, gpointer data)
{
    ChatClient *cc = (ChatClient *)data;
    cc->OnCrossClick();
}