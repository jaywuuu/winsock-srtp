#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <string>
#include <memory>

namespace WinsockTest
{
    class Winsock
    {
    public:
        Winsock();
        ~Winsock();

    private:
        WSADATA wsaData_;
    };

    class AddressInfo
    {
    public:
        AddressInfo(const std::string& addr, const std::string& port, const addrinfo& hints);
        ~AddressInfo();

        AddressInfo(const AddressInfo& addrInfo) = delete;
        AddressInfo& operator=(const AddressInfo& addrInfo) = delete;

        AddressInfo(AddressInfo&& addrInfo);
        AddressInfo& operator=(AddressInfo&& addrInfo);

        const addrinfo* get() const;

    private:
        addrinfo* m_addressInfo;
    };

    class Socket
    {
    public:
        Socket(const std::string& addr, const std::string& port, const addrinfo& hints);
        Socket(SOCKET socket);
        ~Socket();

        bool connect();
        bool disconnect();

        bool bind();
        bool listen();
        std::shared_ptr<Socket> accept();

        bool send(const char* buffer, int len, int flags, int& bytes_sent);
        bool receive(char* buffer, int len, int flags, int& bytes_received);

    private:
        std::string m_addr;
        std::string m_port;
        const addrinfo m_hints;

        SOCKET m_socket;
        std::unique_ptr<AddressInfo> m_addressInfo;

        union
        {
            bool m_isConnected;
            bool m_isBound;
        };
    };
}