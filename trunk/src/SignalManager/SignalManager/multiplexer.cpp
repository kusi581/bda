#include "multiplexer.h"
#include "typedefinitions.h"
#include "config.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>

using namespace std;

multiplexer *multiplexer::m_pInstance = NULL;

bool multiplexer::portLock = false;
map<int, vector<int>> multiplexer::ports;

multiplexer *multiplexer::Instance()
{
    if (!m_pInstance)
        m_pInstance = new multiplexer();

    return m_pInstance;
}

multiplexer::multiplexer()
{
    co.initLog("MUL", true);
}

void multiplexer::init()
{
    cfgGlobal = Config("./SignalManager.cfg");
    cfgGlobal.load();

    this->channels = cfgGlobal.getNumber("channels");
    threads = vector<thread>(channels);

    loadPorts();
}

void multiplexer::start(int channel)
{
    loadPorts();

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
    int reconnectAttempts = 5;
    int recPort;
    int clientPort;
    string localIp = cfgGlobal.getValue("localIp");

    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(localIp.c_str());

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
        recAddr.sin_addr.s_addr = inet_addr(localIp.c_str());
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

            if (ports[channel].size() == 1)
                break;

            for (i = 1;i < ports[channel].size();i++)
            {
                clientPort = ports[channel][i];
                //co.log("Multi " + to_string(recPort) + " to " + to_string(clientPort));
                clientAddr.sin_port = htons(clientPort);
                sendto(senderSocket, udpRecBuffer, sizeof(udpRecBuffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
            }
        }

        reconnectAttempts--;
        close(udpReceiverSocket);
    }
    co.log("Multiplexer stopped: ch" + to_string(channel));
}

void multiplexer::loadPorts()
{
    bool logLoad = false;
    Config cfgChannels("./Channels.cfg");
    Config cfgSlaves("./Slaves.cfg");

    cfgGlobal.load();
    cfgChannels.load();
    cfgSlaves.load();

    map<int, vector<int>> tempPorts;
    int port;

    for (int i = 0; i < cfgGlobal.getNumber("channels"); i++)
    {
        string cKey = "ch" + to_string(i);
        tempPorts[i] = vector<int>(1);
        port = cfgChannels.getNumber(cKey, 2);
        tempPorts[i][0] = port;
        if (logLoad) co.log("ports[" + to_string(i) + "][0] = " + to_string(port));
        if (cfgChannels.getNumber(cKey, 1) != NotRunning)
        {
            port = cfgChannels.getNumber(cKey, 3);
            if (logLoad) co.log("ports[" + to_string(i) + "][1] -> " + to_string(port));
            tempPorts[i].resize(tempPorts[i].size() + 1, port);
        }
    }

    for (int i = 0; i < cfgGlobal.getNumber("slaves"); i++)
    {
        string sKey = "s" + to_string(i);
        int slaveChannel = cfgSlaves.getNumber(sKey, 2);
        if (slaveChannel < 0)
            continue;

        port = cfgSlaves.getNumber(sKey, 1);
        tempPorts[slaveChannel].resize(tempPorts[slaveChannel].size() + 1, port);
        if (logLoad) co.log("ports[" + to_string(i) + "][x] -> " + to_string(port));
    }
    ports = tempPorts;
    co.log("ports loaded");
}

