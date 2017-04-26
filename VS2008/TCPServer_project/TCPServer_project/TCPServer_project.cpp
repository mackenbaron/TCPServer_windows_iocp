#include "stdafx.h"
#include "log.h"
#include "MyTCPServer.h"
#include <stdio.h>
#pragma comment(lib, "TCPServer.lib")
//https://github.com/lihaitao8233/TCPServer_windows_iocp

int main(int argc, char *argv[])
{
    open_log_file("C:\\server", "server", 0);
    set_log_level(LOG_ERROR_LEVEL | LOG_NORMAL_LEVEL | LOG_DEBUG_LEVEL);

    MyTCPServer tcp_server;
    if (argc != 2)
    {
        tcp_server.start(NULL, 5150, 10);
    }
    else
    {
        tcp_server.start(NULL, 5150, atoi(argv[1]));
    }

    getchar();
    tcp_server.stop();

    close_log_file();
    return 0;
}