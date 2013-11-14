#include "ChatLogger.h"

int main(int argc, char** argv) {
    ChatLogger cl("/tmp/log");
    
    cl.Open();
    cl.Log("message to log");
    cl.Close();

    return 0;
}