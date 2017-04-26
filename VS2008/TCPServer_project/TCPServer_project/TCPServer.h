#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_
#include <deque>
#include <set>
#include <winsock2.h>
#include <Windows.h>
#include "thread_mutex_lock.h"


class TCPServer
{
public:
    typedef struct _SERVERTHREADPOOL
    {
        HANDLE thread_handle;

    } SERVER_THREAD_POOL, *P_SERVER_THREAD_POOL;

    typedef struct _PERIODATA
    {
        OVERLAPPED overlapped;
        WSABUF wsa_buf;
        char buf[1024];
        std::deque<char>* p_send_deque;
        std::deque<char>* p_recv_deque;
        int type; //0 接收 1 发送

    } PERIODATA, *P_PERIODATA;

    TCPServer();
    ~TCPServer();

    int start(const char* ip, const int port, int thread_cnt);
    //XP以及之前的系统stop需要和WSASocket、socket进行互斥处理，之后的系统不需要
    int stop();

protected:
    //只能从recv_deque出数据
    //只能往send_deque压数据
    //用户自行定义这个函数，从recv_deque取到接收好的数据，把要发送的数据放入send_deque
    //用户只使用这个函数可以不用关心网络细节
    virtual void work(std::deque<char>& send_deque, std::deque<char>& recv_deque);

private:
    int create_thread_pool(int thread_cnt);
    SOCKET get_listen_socket(const char* ip, const int port, const int backlog); //SOMAXCONN
    static unsigned long WINAPI client_thread(void*);
    static unsigned long WINAPI accept_thread(void*);
    void clear(SOCKET client_socket, P_PERIODATA p_per_io_data);
    P_SERVER_THREAD_POOL m_thread_pool;
    int m_thread_cnt;
    bool m_run_flag;
    HANDLE m_accept_thread_handle;
    SOCKET m_listen_socket;
    HANDLE m_completion_port;
    std::set<SOCKET> m_client_sockets;
    thread_mutex m_mutex;
};

#endif