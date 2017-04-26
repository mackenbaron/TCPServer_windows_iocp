#ifndef TCP_CIENT_H_
#define TCP_CIENT_H_

#include <deque>
#include <winsock2.h>



class TCPClient
{
public:
    TCPClient();
    ~TCPClient();
    int start(const char* ip, const unsigned short port);
    void stop();

protected:
    //只能从recv_deque出数据
    //只能往send_deque压数据
    //用户自行定义这个函数，从recv_deque取到接收好的数据，把要发送的数据放入send_deque
    //用户只使用这个函数可以不用关心网络细节
    virtual void work(std::deque<char>& send_deque, std::deque<char>& recv_deque);

private:
    static unsigned long WINAPI thread_func(void*);
    int connect_to_server(const char* ip, const unsigned short port);
    int connect_retry(const SOCKET socket, const struct sockaddr* addr, const int alen, const int connect_cnt);
    int read_from_server();
    int write_to_server();


private:
    unsigned int m_port;
    char m_server_ip[16];
    bool m_run_flag;
    SOCKET m_socket;
    HANDLE m_thread_handle;
    bool m_write_event;
    std::deque<char> m_recv_queque;
    std::deque<char> m_send_queque;
};

#endif
