#include "multiplexer.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

multiplexer::multiplexer()
{
    co.initLog("MUL", true);
}

void multiplexer::init(int checkInterval)
{
    this->checkInterval = checkInterval;
}

void multiplexer::start()
{
    int channel = 0;

    threads[channel] = thread(&multiplexer::startMultiplexing, this, channel);
}

void multiplexer::stop()
{

}

void multiplexer::startMultiplexing(int channel)
{
    int udpReceiverSocket, bytes_read, senderSocket;
    unsigned char udpRecBuffer[512];
    struct sockaddr_in recAddr, hwServerAddr;
    unsigned int length;

    // todo; get ports from config
    int recPort = 12222;

    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    senderSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    while (true)
    {
        udpReceiverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (udpReceiverSocket == -1)
        {
            co.log("Error: udp socket create");
            break;
        }

        recAddr.sin_family = AF_INET;
        recAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        recAddr.sin_port = htons(recPort);

        if (bind(udpReceiverSocket, (struct sockaddr*)&recAddr, sizeof(recAddr)) == -1)
        {
            co.log("Error while binding socket to port (udp)");
            break;
        }

        while (true) {
            bytes_read = recvfrom(udpReceiverSocket, udpRecBuffer, sizeof(udpRecBuffer), 0, (struct sockaddr*)&hwServerAddr, &length);
            if (bytes_read < 0){
                co.log("Error while reading from udp socket.");
                break;
            }

            // todo: read from config where to forward
            clientAddr.sin_port = htons(12223);
            sendto(senderSocket, udpRecBuffer, sizeof(udpRecBuffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));

            clientAddr.sin_port = htons(12224);
            sendto(senderSocket, udpRecBuffer, sizeof(udpRecBuffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        }

        close(udpReceiverSocket);
    }
}

