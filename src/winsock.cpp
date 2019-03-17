#include <stdio.h>
#include <stdexcept>

#pragma comment(lib, "Ws2_32.lib")

#include "winsock.h"

using namespace WinsockTest;

Winsock::Winsock()
{
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData_);
    if (result != 0)
        throw std::runtime_error("Call to WSAStartup failed.");
}

Winsock::~Winsock()
{
    WSACleanup();
}

AddressInfo::AddressInfo(const std::string& addr, const std::string& port, const addrinfo& hints)
{
    addrinfo* addressInfo = nullptr;

    int result = getaddrinfo(addr.size() ? addr.c_str() : nullptr, port.c_str(), &hints, &addressInfo);

    if (result != 0)
        throw std::runtime_error("Cannot get address information from address and port.");

    m_addressInfo = addressInfo;
}

AddressInfo::~AddressInfo()
{
    if (m_addressInfo)
        freeaddrinfo(m_addressInfo);
}

AddressInfo::AddressInfo(AddressInfo&& addrInfo) :
    m_addressInfo(addrInfo.m_addressInfo)
{
    addrInfo.m_addressInfo = nullptr;
}

AddressInfo& AddressInfo::operator=(AddressInfo&& addrInfo)
{
    if (this == &addrInfo)
        return *this;

    if (m_addressInfo)
        freeaddrinfo(m_addressInfo);

    m_addressInfo = addrInfo.m_addressInfo;
    addrInfo.m_addressInfo = nullptr;

    return *this;
}

const addrinfo* AddressInfo::get() const
{
    return m_addressInfo;
}

Socket::Socket(const std::string& addr, const std::string& port, const addrinfo& hints) :
    m_addr(addr),
    m_port(port),
    m_hints(hints),
    m_isConnected(false)
{
    m_addressInfo = std::make_unique<AddressInfo>(addr, port, hints);
    auto addressInfo = m_addressInfo->get();

    SOCKET sock = INVALID_SOCKET;
    sock = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);

    if (sock == INVALID_SOCKET)
        throw std::runtime_error("Cannot create socket from address info.  Error: " + WSAGetLastError());

    m_socket = sock;
}

Socket::Socket(SOCKET socket) :
    m_socket(socket),
    m_hints({ 0 })
{
}

Socket::~Socket()
{
    disconnect();

    m_addressInfo = nullptr;

    if (m_socket != INVALID_SOCKET)
        closesocket(m_socket);
}

bool Socket::connect()
{
    if (m_isConnected)
        return true;

    if (!m_addressInfo)
        m_addressInfo = std::make_unique<AddressInfo>(m_addr, m_port, m_hints);

    if (m_addressInfo)
    {
        auto addrInfoIt = m_addressInfo->get();
        int result = SOCKET_ERROR;

        while (addrInfoIt)
        {
            result = ::connect(m_socket, addrInfoIt->ai_addr, static_cast<int>(addrInfoIt->ai_addrlen));

            // If connection fails, try next address
            if (result == SOCKET_ERROR)
                addrInfoIt = addrInfoIt->ai_next;
            else
                break;
        }

        if (result == SOCKET_ERROR)
            return false;

        m_isConnected = true;

        // Free resources no longer needed
        m_addressInfo = nullptr;
    }
    else
    {
        return false;
    }

    return true;
}

bool Socket::disconnect()
{
    if (!m_isConnected)
        return true;

    if (shutdown(m_socket, SD_SEND) == SOCKET_ERROR)
        return false;
        
    m_isConnected = false;

    return true;
}

bool Socket::bind()
{
    if (m_isBound)
        return true;

    if (!m_addressInfo)
        m_addressInfo = std::make_unique<AddressInfo>(m_addr, m_port, m_hints);

    if (m_addressInfo)
    {
        auto addrInfo = m_addressInfo->get();
        int result = ::bind(m_socket, addrInfo->ai_addr, static_cast<int>(addrInfo->ai_addrlen));

        if (result == SOCKET_ERROR)
            return false;

        m_addressInfo = nullptr;

        m_isBound = true;
    }
    else
    {
        return false;
    }
   
    return true;
}

bool Socket::listen()
{
    if (!m_isBound)
        throw std::runtime_error("Failed to listen on socket.  Not yet bound.");

    int result = ::listen(m_socket, SOMAXCONN);

    if (result == SOCKET_ERROR)
        return false;

    return true;
}

std::shared_ptr<Socket> Socket::accept()
{
    SOCKET sock = ::accept(m_socket, nullptr, nullptr);
    if (sock == INVALID_SOCKET)
        return nullptr;

    return std::make_shared<Socket>(sock);
}

bool Socket::send(const char* buffer, int len, int flags, int& bytes_sent)
{
    int res = ::send(m_socket, buffer, len, flags);

    if (res == SOCKET_ERROR)
        return false;

    bytes_sent = res;

    return true;
}

bool Socket::receive(char* buffer, int len, int flags, int& bytes_received)
{
    int res = recv(m_socket, buffer, len, flags);

    if (res == SOCKET_ERROR)
        return false;

    bytes_received = res;

    return true;
}