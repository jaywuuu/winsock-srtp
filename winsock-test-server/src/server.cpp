#include "winsock.h"

#include <iostream>
#include <chrono>
#include <thread>

#pragma comment(lib, "winsock-test.lib")

using namespace WinsockTest;

int main(int argc, char *argv[])
{
    std::cout << "Winsock Test Server" << std::endl;
    std::cout << "Initializing winsock." << std::endl;
    Winsock wsock{};

    std::cout << "Creating socket." << std::endl;
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    Socket sock{ "", "27015", hints };
    sock.bind();

    bool listenSuccess = false;
    bool serverRunning = true;

    while (serverRunning)
    {
        std::cout << "Listening on socket." << std::endl;
        do {
            listenSuccess = sock.listen();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } while (!listenSuccess);

        std::shared_ptr<Socket> clientSocket = sock.accept();

        if (!clientSocket)
        {
            std::cout << "Failed to accept client connection." << std::endl;
            return 0;
        }

        std::cout << "Successfully accepted client connection." << std::endl;

        char buff[512];
        memset(buff, 0, 512);

        int receivedBytes = 0;
        bool receiveStatus = false;

        do {
            receiveStatus = clientSocket->receive(buff, 512, 0, receivedBytes);

            if (!receiveStatus)
                std::cout << "Failed to receive from client." << std::endl;
            else
                std::cout << "Received " << receivedBytes << " bytes of data" << std::endl;

            if (receivedBytes > 0)
                std::cout << "Message: " << buff << std::endl;

        } while (receiveStatus && receivedBytes != 0);
    }

    return 0;
}