#include "multiplexer.h"
#include "config.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>

using namespace std;

multiplexer::multiplexer()
{
    co.initLog("MUL", true);
}

void multiplexer::init()
{
    Config cfg("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/SignalManager.cfg");
    cfg.load();

    this->checkInterval = cfg.getNumber("multiplexCheckInterval");
    this->channels = cfg.getNumber("channels");
    threads = vector<thread>(channels);

    loadPorts();
}

void multiplexer::start(int channel)
{
    if (ports[channel].size() == 0 || ports[channel][0] == 0)
    {
        co.log("multiplexer " + to_string(channel) + " not started, invalid configuration");
    }
    else if (threads[channel].joinable())
    {
        co.log("multiplexer " + to_string(channel) + " is already running");
    }
    else
    {
        threads[channel] = thread(&multiplexer::startMultiplexing, this, channel);
    }
}

void multiplexer::stop()
{
    int c = 0;
    for (vector<thread>::iterator i = threads.begin(); i != threads.end(); i++)
    {
        co.log("Thread " + to_string(c) + " -> " + ((*i).joinable() ? "running" : "not running"));
        c++;
    }

    co.log("all multiplexing threads stopped");
}

void multiplexer::startMultiplexing(int channel)
{
    co.log("Multiplexer started: ch" + to_string(channel));

    int udpReceiverSocket, bytes_read, senderSocket, i;
    unsigned char udpRecBuffer[512];
    struct sockaddr_in recAddr, hwServerAddr;
    unsigned int length;
    bool isRunning = true;
    time_t lastCheck = time(0);
    int reconnectAttempts = 10;
    int recPort;
    int clientPort;

    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    senderSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    while (reconnectAttempts > 0)
    {
        udpReceiverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (udpReceiverSocket == -1)
        {
            co.log("Error: udp socket create");
            break;
        }

        if (ports[channel][0] == 0)
            break;
        recPort = ports[channel][0];

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
            if (bytes_read < 0)
            {
                co.log("Error while reading from udp socket.");
                break;
            }

            if (ports[channel][0] == 0)
                break;

            for (i = 1;i < channels;i++)
            {
                clientPort = ports[channel][i];
                if (clientPort == 0)
                    continue;

                clientAddr.sin_port = htons(clientPort);
                sendto(senderSocket, udpRecBuffer, sizeof(udpRecBuffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
            }

            if (time(0) - lastCheck > checkInterval)
            {
                lastCheck = time(0);
                thread(&multiplexer::loadPorts, this).detach();
            }
        }

        reconnectAttempts--;
        close(udpReceiverSocket);
    }
    co.log("Multiplexer stopped: ch" + to_string(channel));
}

void multiplexer::loadPorts()
{
    map<int, vector<int>> tempPorts;
    int currentChannel = 0, currentPart = 0;
    string channelKey = co.getChannelKey(currentChannel);
    Config cfg("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/Multiplexer.cfg");
    cfg.load();

    while (cfg.keyExists(channelKey))
    {
        vector<string> parts = co.split(cfg.getValue(channelKey), ',');

        if (parts[0] != "")
        {
            tempPorts[currentChannel] = vector<int>(channels);
            for (currentPart = 0;currentPart < parts.size();currentPart++)
            {
                tempPorts[currentChannel][currentPart] = parts[currentPart].length() > 0 ? stoi(parts[currentPart]) : 0;
            }
        }
        currentChannel += 1;
        channelKey = co.getChannelKey(currentChannel);
    }
    ports = tempPorts;
}

