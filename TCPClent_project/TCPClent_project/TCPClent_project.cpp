#include "stdafx.h"
#include "MyTCPClient.h"
#include "log.h"

int  main()
{
    open_log_file("D:\\client", "client", 0);
    set_log_level(LOG_ERROR_LEVEL | LOG_NORMAL_LEVEL | LOG_DEBUG_LEVEL);

    MyTCPClient my_tcp_client;
    my_tcp_client.start("10.232.31.9", 5150);
    Sleep(1000000000);



    close_log_file();
    return 0;
}



