#include "stdafx.h"
#include "log.h"
#include "MyTCPServer.h"
#include <stdio.h>
#pragma comment(lib, "TCPServer.lib")
//https://github.com/lihaitao8233/TCPServer-windows-iocp-.git

int main(int argc, char *argv[])
{
    open_log_file("D:\\server", "server", 0);
    set_log_level(LOG_ERROR_LEVEL | LOG_NORMAL_LEVEL | LOG_DEBUG_LEVEL);

    MyTCPServer tcp_server;
    tcp_server.start(NULL, 5150, 10);
    getchar();

    close_log_file();
    return 0;
}