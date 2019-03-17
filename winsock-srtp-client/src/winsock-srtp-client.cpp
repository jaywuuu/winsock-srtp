#include <iostream>
#include <thread>

#include "winsock.h"
#include "srtplib.h"

#pragma comment(lib, "srtp2.lib")

using namespace WinsockTest;

static bool g_recieve = true;

void recv_routine(Socket* sock, SRTP* srtp)
{
    do {
        SRTP::srtp_msg msg = {};
        int recvBytes = 0;
        sock->receive((char*)&msg, sizeof(SRTP::srtp_msg), 0, recvBytes);

        if (!recvBytes || recvBytes == 0) {
            std::cout << "Connection disconnected." << std::endl;
            break;
        }

        std::cout << "Received " << recvBytes << " bytes." << std::endl;
        std::cout << "{" << std::endl;
        std::cout << "    msg.body_size: " << msg.body_size << std::endl;
        std::cout << "    msg.body: " << msg.body << std::endl;
        std::cout << "}" << std::endl;

        if (srtp)
        {
            char buff[SRTP::max_msg_length];
            int msgLen = 0;
            if (!srtp->Unprotect(msg, buff, msgLen))
                std::cout << "Failed to unprotect message." << std::endl;
            else
                std::cout << "Unprotected message: " << buff << std::endl;
        }
    } while (g_recieve);
}

int main(int argc, char* argv[])
{
    std::cout << "########################################" << std::endl;
    std::cout << "##.......    SRTP Client  ............##" << std::endl;
    std::cout << "########################################" << std::endl;

    std::cout << "Winsock Test Client" << std::endl;
    std::cout << "Initializing winsock." << std::endl;
    Winsock wsock{};

    std::cout << "Creating socket." << std::endl;
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::string ip = (argc >= 2) ? argv[1] : "127.0.0.1";
    std::string port = (argc >= 3) ? argv[2] : "27015";

    Socket sock{ ip, port, hints };

    std::cout << "Trying to connect to server." << std::endl;

    if (!sock.connect())
    {
        std::cout << "Failed to connect to server." << std::endl;
        return 0;
    }

    std::cout << "Successfully connected to server." << std::endl;

    char buff[512];

    bool running = true;

    std::thread recv_thread(recv_routine, &sock, nullptr);

    do {
        std::string msg;

        std::cin >> msg;

        if (msg == ".quit")
            break;

        int len = static_cast<int>(msg.size() > 512 ? 512 : msg.size());

        memset(buff, 0, 512);
        memcpy(buff, msg.c_str(), len);

        int bytesSent = 0;

        std::cout << "Sending message: " << buff << std::endl;

        if (!sock.send(buff, len, 0, bytesSent))
        {
            std::cout << "Failed to send message.  Closing connection." << std::endl;
            running = false;
            break;
        }
        else
            std::cout << "Sent " << len << " bytes of data." << std::endl;
    } while (running);

    g_recieve = false;
    sock.disconnect();

    recv_thread.join();

    return 0;
}