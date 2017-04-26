#include "stdafx.h"
#include "MyTCPClient.h"
#include "log.h"
//https://github.com/lihaitao8233/TCPServer_windows_iocp

int  main()
{
    open_log_file("C:\\client", "client", 0);
    set_log_level(LOG_ERROR_LEVEL | LOG_NORMAL_LEVEL | LOG_DEBUG_LEVEL);

    MyTCPClient my_tcp_client;
    my_tcp_client.start("123.123.123.123", 5150);//填入正确的IP地址
    getchar();
    my_tcp_client.stop();

    close_log_file();
    return 0;
}



