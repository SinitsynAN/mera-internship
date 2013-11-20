#include <gtk/gtk.h>

GtkWidget *window;
GtkWidget *table;
GtkWidget *button;
GtkWidget *entry;
GtkWidget *textView;
GtkWidget *scrolledWindow;

void onClick(GtkWidget *widget, gpointer data)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    const gchar *entryText;
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
    gtk_text_buffer_get_end_iter(buffer, &iter);
    entryText = gtk_entry_get_text(GTK_ENTRY(entry));
    
    gtk_text_buffer_insert(buffer, &iter, entryText, -1);
    g_print("> %s\n", entryText);
}

int main(int argc, char *argv[])
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
    gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(onClick), NULL);
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
    
    gtk_main ();
    //gtk_main_iteration();
    
    return 0;
}