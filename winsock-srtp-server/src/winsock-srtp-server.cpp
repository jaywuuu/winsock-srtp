#include <iostream>

#include <chrono>
#include <thread>

#include "winsock.h"
#include "srtplib.h"

#pragma comment(lib, "srtp2.lib")

using namespace WinsockTest;

int main(int argc, char* argv[])
{
    std::cout << "########################################" << std::endl;
    std::cout << "##.......    SRTP Server  ............##" << std::endl;
    std::cout << "########################################" << std::endl;

    std::cout << "Initializing winsock." << std::endl;
    Winsock wsock{};

    std::cout << "Creating socket." << std::endl;
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    std::string port = (argc >= 2) ? argv[1] : "27015";

    Socket sock{ "", port, hints };
    sock.bind();

    std::cout << "Initialize libstrp" << std::endl;
    SRTP srtp{};
    srtp.CreateSession();

    bool listenSuccess = false;
    bool serverRunning = true;

    while (serverRunning)
    {
        std::cout << "Listening on socket." << std::endl;
        listenSuccess = sock.listen();

        if (!listenSuccess) 
        {
            std::cout << "Failed to listen on socket." << std::endl;
            return 0;
        }

        std::shared_ptr<Socket> clientSocket = sock.accept();

        if (!clientSocket)
        {
            std::cout << "Failed to accept client connection." << std::endl;
            return 0;
        }

        std::cout << "Successfully accepted client connection." << std::endl;

        char buff[512];

        int receivedBytes = 0;
        bool receiveStatus = false;

        SRTP::srtp_msg msg = {};
        srtp.InitHeader(msg.header);

        do {
            memset(buff, 0, 512);

            receiveStatus = clientSocket->receive(buff, 512, 0, receivedBytes);

            if (!receiveStatus || receivedBytes == 0) {
                std::cout << "Client disconnected." << std::endl;
                break;
            }

            std::cout << "Successfully received " << receivedBytes << " bytes from client." << std::endl;
            std::cout << "msg: " << buff << std::endl;

            // wrap received bytes into a srtp packet and send it back.

            memset(msg.body, 0, sizeof(char)*SRTP::max_msg_length);

            if (!srtp.Protect(buff, receivedBytes, msg))
                std::cout << "Failed to protect message." << std::endl;

            std::cout << "Sending response back using SRTP." << std::endl;
            int bytesSent = 0;
            clientSocket->send((char*)&msg, SRTP_MSG_HEADER_SIZE + (int)msg.body_size, 0, bytesSent);

        } while (receiveStatus && receivedBytes != 0);
    }

    return 0;
}