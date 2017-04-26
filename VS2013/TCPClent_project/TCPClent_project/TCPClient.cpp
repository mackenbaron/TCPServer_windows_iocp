#include "StdAfx.h"
#include "TCPClient.h"
#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

TCPClient::TCPClient() :
           m_socket(-1),
           m_run_flag(false),
           m_port(5150),
           m_write_event(true)
{
    memset(m_server_ip, 0, sizeof(m_server_ip));
    WSADATA wsaData;
    WSAStartup(0x0202, &wsaData);
}

TCPClient::~TCPClient()
{
    WSACleanup();
    stop();
}

void TCPClient::stop()
{
    if (m_run_flag)
    {
        m_run_flag = false;
        shutdown(m_socket, SD_BOTH);
        WaitForSingleObject(m_thread_handle, INFINITE);
        CloseHandle(m_thread_handle);
        m_thread_handle = NULL;
    }

    if (INVALID_SOCKET != m_socket)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    m_recv_queque.clear();
    m_send_queque.clear();
}

int TCPClient::connect_to_server(const char* ip, const unsigned short port)
{
    m_recv_queque.clear();
    m_send_queque.clear();
    if (strlen(ip) > 15)
    {
        LOGE("ip error = %s", ip);
        return -1;
    }

    if (INVALID_SOCKET != m_socket)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    m_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == m_socket)
    {
        LOGE("WSASocket error");
        return -2;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip);
    m_port = port;
    strcpy(m_server_ip, ip);

    unsigned long ul = 1;
    if (ioctlsocket(m_socket, FIONBIO, &ul) == SOCKET_ERROR)
    {
        LOGE("ioctlsocket error");
        return -3;
    }

    while (m_run_flag)
    {
        if (connect(m_socket, (struct sockaddr*)(&server_address), sizeof(struct sockaddr)) != 0)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                fd_set write_set;
                struct timeval tv;
                FD_ZERO(&write_set);
                FD_SET(m_socket, &write_set);
                tv.tv_sec = 0;
                tv.tv_usec = 1000 * 1000;
                int ret = select(m_socket + 1, NULL, &write_set, NULL, &tv);
                if (-1 == ret)
                {
                    LOGE("select error");
                    return -7;
                }

                if (0 == ret)  return -6;

                int error = -1;
                int len = sizeof(int);
                if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == -1)
                {
                    LOGE("getsockopt error");
                    return -8;
                }

                if (0 == error)
                {
                    LOGN("connect to ip = %s port = %d socket = %d success", m_server_ip, m_port, m_socket);
                    return 0;
                }
                else
                {
                    LOGN("connect fail");
                    return -4;
                }
            }
            else
            {
                LOGE("connect error");
                return -5;
            }
        }
        else
        {
            break;
        }
    }

    LOGN("connect to ip = %s port = %d socket = %d success", m_server_ip, m_port, m_socket);
    return 0;
}

int TCPClient::start(const char* ip, const unsigned short port)
{
    stop();

    m_run_flag = true;
    if (connect_to_server(ip, port) < 0)
    {
        m_run_flag = false;
        return -1;
    }

    m_thread_handle = CreateThread(NULL, 0, thread_func, this, 0, NULL);
    if (NULL == m_thread_handle)
    {
        LOGE("CreateThread error");
        m_run_flag = false;
        return -2;
    }

    return 0;
}

unsigned long WINAPI TCPClient::thread_func(void* arg)
{
    TCPClient* p_tcp_client = (TCPClient*)arg;
    fd_set readset;
    FD_ZERO(&readset);
    fd_set writeset;
    FD_ZERO(&writeset);
    //struct timeval tv;
    int ret = -1;

    while(p_tcp_client->m_run_flag)
    {
        FD_ZERO(&readset);
        FD_SET(p_tcp_client->m_socket, &readset);//FD_CLR
        //linux下select返回后tv值不确定 
        //tv.tv_sec = 0; 
        //tv.tv_usec = 2000000;
        if (p_tcp_client->m_write_event)
        {
            FD_ZERO(&writeset);
            FD_SET(p_tcp_client->m_socket, &writeset);
            ret = select(p_tcp_client->m_socket + 1, &readset, &writeset, NULL, NULL/*&tv*/);
        }
        else
        {
            ret = select(p_tcp_client->m_socket + 1, &readset, NULL, NULL, NULL/*&tv*/);
        }

        if (ret < 0)
        {
            //if (errno != EINTR)  { LOGE("select error"); continue; }
            //else  continue;
            LOGE("select error"); continue;
        }

        ret = 0;
        if (FD_ISSET(p_tcp_client->m_socket, &readset))
        {
            ret = p_tcp_client->read_from_server();
        }

        p_tcp_client->work(p_tcp_client->m_send_queque, p_tcp_client->m_recv_queque);

        if (FD_ISSET(p_tcp_client->m_socket, &writeset))
        {
            ret = p_tcp_client->write_to_server();
        }

        if (ret < 0)
        {
            while (1)
            {
                if (p_tcp_client->connect_to_server(p_tcp_client->m_server_ip, p_tcp_client->m_port) < 0)  Sleep(1000);
                else  break;
            }

            p_tcp_client->m_write_event = true;
        }
        else
        {
            if (p_tcp_client->m_send_queque.size() > 0)  p_tcp_client->m_write_event = true;
            else p_tcp_client->m_write_event = false;
        }
    }

    return 0;
}

int TCPClient::read_from_server()
{
    char buf[1024] = { 0 };
    int ret = recv(m_socket, buf, sizeof(buf), 0);
    if (ret < 0)
    {
        if (WSAGetLastError() == WSAEWOULDBLOCK)  LOGD("read error");
        else
        {
            LOGE("read error");
            shutdown(m_socket, SD_BOTH);
            if (INVALID_SOCKET != m_socket)
            {
                closesocket(m_socket);
                m_socket = INVALID_SOCKET;
            }

            return -1;
        }
    }
    else if (0 == ret)
    {
        LOGN("server %d closed", m_socket);
        shutdown(m_socket, SD_BOTH);
        if (INVALID_SOCKET != m_socket)
        {
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
        }

        return -2;
    }
    else
    {
        for (int i = 0; i < ret; i++)
        {
            m_recv_queque.push_back(buf[i]);
        }
    }

    return 0;
}

void TCPClient::work(std::deque<char>& send_deque, std::deque<char>& recv_deque)
{

}

int TCPClient::write_to_server()
{
    char buf[1024] = { 0 };
    int num = 0;
    if (m_send_queque.size() >= sizeof(buf))  num = sizeof(buf);
    else  num = m_send_queque.size();
    if (0 == num)  return 0;
    for (int i = 0; i < num; i++)
    {
        buf[i] = m_send_queque.front();
        m_send_queque.pop_front();
    }

    int ret = send(m_socket, buf, num, 0);
    if (ret < 0)
    {
        if (WSAGetLastError() == WSAEWOULDBLOCK)  LOGD("write error");
        else
        {
            LOGE("write error");
            shutdown(m_socket, SD_BOTH);
            if (INVALID_SOCKET != m_socket)
            {
                closesocket(m_socket);
                m_socket = INVALID_SOCKET;
            }

            return -1;
        }
    }
    else
    {
        for (int i = num - 1; i >= ret; i--)
        {
            m_send_queque.push_front(buf[i]);
        }
    }

    return 0;
}
