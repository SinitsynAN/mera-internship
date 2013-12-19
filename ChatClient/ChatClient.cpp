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
    memset(&clientAddress, 0, sizeof(struct sockaddr_in));
    memset(&serverAddress, 0, sizeof(struct sockaddr_in));
    isSocketForServerCreated = false;
    isSocketForServerBinded = false;
    isServerAddressCorrect = false;
    
    config.Init();
    
    clientNickname = new char[7];
    strcpy(clientNickname, "Anonym");
    
    memset(buffer, 0, BUFFER_SIZE * sizeof(char));
    bytesReceived = 0;
    
    messageManager = new MessageManager(BUFFER_SIZE);
    confirmationCount = 0;
    
    serviceMessages.Fill();
    
    logger = new Logger(LOG_PATH);
}

ChatClient::~ChatClient()
{
    close(socketForServer);
    delete [] clientNickname;
    //delete messageManager; // <--- ??
    delete logger;
}

void ChatClient::SetNickname(char *nickname)
{
    delete [] clientNickname;
    
    int len = strlen(nickname);
    clientNickname = new char[len + 1];
    strcpy(clientNickname, nickname);
}

bool ChatClient::IsSocketForServerValid()
{
    return isSocketForServerCreated && isSocketForServerBinded && isServerAddressCorrect;
}

void ChatClient::InitSocketForServer()
{
    if (isSocketForServerCreated) {
        close(socketForServer);
        isSocketForServerCreated = false;
    }
    
    socketForServer = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketForServer < 0) {
        logger->Log("socketForServer hasn't been created");
        exit(1);
    }
    isSocketForServerCreated = true;
    
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(config.servPort)); 
    if (inet_pton(AF_INET, config.servAddr, &serverAddress.sin_addr) <= 0) {
        printf("\nInvalid IP-address.\n");
        logger->Log("Invalid IP-address");
        isServerAddressCorrect = false;
        return;
    }
    isServerAddressCorrect = true;
        
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(atoi(config.cliPort));
    clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socketForServer, (struct sockaddr *) &clientAddress, sizeof(clientAddress)) < 0) {
        printf("\nSocket hasn't been binded.\n");
        logger->Log("socketForServer hasn't been binded");
        isSocketForServerBinded = false;
        return;
    }
    isSocketForServerBinded = true;
    fcntl(socketForServer, F_SETFL, O_NONBLOCK);
}

void ChatClient::Start()
{
    OnSettingsButtonClick();
    InitSocketForServer();
    if (IsSocketForServerValid()) {
        printf("\nClient has been successfully started\n");
        logger->Log("Client has been successfully started");
        Work();
    }
    else
        logger->Log("socketForServer hasn't been initialized");
}

void ChatClient::Send(char *message)
{
    int msgSize = strlen(message) + 1;
    if (msgSize > 1) {
        sendto(socketForServer, message, msgSize, 0, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    }
}

void ChatClient::Work()
{
    if (IsSocketForServerValid())
        messageManager->AddToQueue(clientNickname);
    
    while(1) {
        if (IsSocketForServerValid()) {
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(socketForServer, &readSet);

            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(socketForServer, &writeSet);

            timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;

            int maxFd = socketForServer;
            int selectResult = select(maxFd + 1, &readSet, &writeSet, NULL, &timeout);
            if (selectResult < 0) {
                logger->Log("select error");
                break;
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
                else if (res == 3)
                    confirmationCount--;
            }

            if (FD_ISSET(socketForServer, &readSet)) {
                bytesReceived = recvfrom(socketForServer, buffer, BUFFER_SIZE, 0, NULL, NULL);
                buffer[bytesReceived] = '\0';

                if (strcmp(buffer, serviceMessages.in[0]) == 0) {
                    confirmationCount++;
                }
                else if (strcmp(buffer, serviceMessages.in[1]) == 0) {
                    if (!messageManager->IsQueueFull())
                        messageManager->AddToQueue(serviceMessages.out[0]);
                }
                else {
                    ShowIncomingMessage(buffer);
                }
            }
        }
        
        while (gtk_events_pending())
            gtk_main_iteration();
    }
}

//---------------------------------------------------

void ChatClient::InitGraphics(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "ChatClient"); 
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(ExitButtonClickProxy), NULL); 
    
    table = gtk_table_new(15, 20, FALSE);
    gtk_container_add(GTK_CONTAINER(window), table);
    
    button = gtk_button_new_with_label("Settings");
    gtk_table_attach(GTK_TABLE(table), button, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
    gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(SettingsButtonClickProxy), this);
    
    button = gtk_button_new_with_label("Exit");
    gtk_table_attach(GTK_TABLE(table), button, 19, 20, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
    gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(ExitButtonClickProxy), this);
    
    textView = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textView), FALSE);
    
    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), textView);
    gtk_table_attach_defaults(GTK_TABLE(table), scrolledWindow, 0, 20, 1, 14);
    
    entry = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 0, 19, 14, 15);
    gtk_signal_connect(GTK_OBJECT(entry), "activate", GTK_SIGNAL_FUNC(SendButtonClickProxy), this);
    
    button = gtk_button_new_with_label("Send");
    gtk_table_attach(GTK_TABLE(table), button, 19, 20, 14, 15, GTK_SHRINK, GTK_SHRINK, 0 ,0);
    gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(SendButtonClickProxy), this);

    gtk_widget_show_all(window);
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

void ChatClient::OnSettingsButtonClick()
{
    GtkWidget *dialog;
    GtkWidget *table;
    GtkWidget *label;
    GtkWidget *servAddrEntry;
    GtkWidget *servPortEntry;
    GtkWidget *cliPortEntry;
    GtkWidget *contentArea;

    dialog = gtk_dialog_new_with_buttons ("Settings",
                                         NULL,
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_NONE,
                                         NULL);

    contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    table = gtk_table_new(6, 1, TRUE);
    gtk_container_add(GTK_CONTAINER(contentArea), table);
    
    label = gtk_label_new("Server IP address");
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);

    servAddrEntry = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), servAddrEntry, 0, 1, 1, 2);
    
    label = gtk_label_new("Server port");
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 2, 3);

    servPortEntry = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), servPortEntry, 0, 1, 3, 4);
    
    label = gtk_label_new("Client port");
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 4, 5);

    cliPortEntry = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), cliPortEntry, 0, 1, 5, 6);
    
    gtk_widget_show_all(contentArea);

    gtk_dialog_run(GTK_DIALOG(dialog));
    
    strcpy(config.servAddr, (char *) gtk_entry_get_text(GTK_ENTRY(servAddrEntry)));
    strcpy(config.servPort, (char *) gtk_entry_get_text(GTK_ENTRY(servPortEntry)));
    strcpy(config.cliPort, (char *) gtk_entry_get_text(GTK_ENTRY(cliPortEntry)));
    
    gtk_widget_destroy(dialog);
}

void ChatClient::SettingsButtonClickProxy(GtkWidget* widget, gpointer data)
{
    ChatClient *cc = (ChatClient *)data;
    
    if (cc->IsSocketForServerValid())
        cc->Send(cc->serviceMessages.out[1]);
    cc->OnSettingsButtonClick();
    cc->InitSocketForServer();
    
    if (cc->IsSocketForServerValid())
        cc->messageManager->AddToQueue(cc->clientNickname);
}

void ChatClient::OnSendButtonClick()
{
    const gchar *entryText;   
    entryText = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (!messageManager->IsQueueFull()) {
        if (strlen((char *)entryText) > 0)
            messageManager->AddToQueue((char *)entryText);
    }
    else {
        printf("\nMessage queue is full\n");
        logger->Log("Message queue is full");
    }
    
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

void ChatClient::SendButtonClickProxy(GtkWidget* widget, gpointer data)
{
    ChatClient *cc = (ChatClient *)data;
    cc->OnSendButtonClick();
}

void ChatClient::OnExitButtonClick()
{
    Send(serviceMessages.out[1]);
    exit(0);
}

void ChatClient::ExitButtonClickProxy(GtkWidget* widget, gpointer data)
{
    ChatClient *cc = (ChatClient *)data;
    cc->OnExitButtonClick();
}