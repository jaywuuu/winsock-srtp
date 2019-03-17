#include "winsock.h"

#include <iostream>

#pragma comment(lib, "winsock-test.lib")

using namespace WinsockTest;

int main(int argc, char *argv[])
{
    std::cout << "Winsock Test Client" << std::endl;
    std::cout << "Initializing winsock." << std::endl;
    Winsock wsock{};

    std::cout << "Creating socket." << std::endl;
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::string ip = argv[1];
    std::string port = argv[2];
    std::string msg = argv[3];

    Socket sock{ ip, port, hints };

    std::cout << "Trying to connect to server." << std::endl;
    
    if (!sock.connect())
    {
        std::cout << "Failed to connect to server." << std::endl;
        return 0;
    }

    std::cout << "Successfully connected to server." << std::endl;

    char buff[512];
    memset(buff, 0, 512);

    int len = static_cast<int>(msg.size() > 512 ? 512 : msg.size());

    memcpy(buff, msg.c_str(), len);

    int bytesSent = 0;

    std::cout << "Sending message: " << buff << std::endl;

    if (!sock.send(buff, len, 0, bytesSent))
        std::cout << "Failed to send message." << std::endl;
    else
        std::cout << "Sent " << len << " bytes of data." << std::endl;

    return 0;
}