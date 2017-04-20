#include "dspmanager.h"
#include "common.h"
#include "config.h"
#include "commandhandler.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>

int tcpSocket;
struct sockaddr_in socketAddr;
char receiveBuffer[512];
int bytes_read;
unsigned int socketAddrSize = sizeof(socketAddr);
Config cfgGlobal;
Config cfgChannels;
int nrOfConnections = 5; // todo: configurable in global

dspManager::dspManager()
{
    co.initLog("dsp", true);
    isRunning = false;
    setupOk = false;
    cfgGlobal = Config("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/SignalManager.cfg");
    cfgGlobal.load();

    cfgChannels = Config("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/DspMapping.cfg");
}

void dspManager::startListener()
{
    if (!setupOk){
        co.log("Listener not started, there was an error during setup");
        return;
    }

    if (isRunning)
    {
        co.log("Listener not started, already running");
        return;
    }

    isRunning = true;
    std::thread thr(&dspManager::waitForConnection, this);
    std::swap(thr, listenThread);
}

void dspManager::stopListener()
{
    close(tcpSocket);
    isRunning = false;
    listenThread.detach();
}

void dspManager::setupSocket()
{
    tcpSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (tcpSocket == -1)
    {
        setupOk = false;
        co.log("Tcp socket error: socket()");
        return;
    }
    co.log("tcp Socket created");

    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socketAddr.sin_port = htons(cfgGlobal.getNumber("tcpListenPort"));

    if (bind(tcpSocket, (struct sockaddr *) &socketAddr, socketAddrSize) < 0)
    {
        setupOk = false;
        co.log("Tcp socket error: bind()");
        return;
    }

    if (listen(tcpSocket, nrOfConnections) < 0)
    {
        setupOk = false;
        co.log("Tcp socket error: listen()");
        return;
    }
    setupOk = true;
    co.log("setup ok");
}

void dspManager::waitForConnection()
{
    std::thread cThr;
    int socketClient;
    struct sockaddr_in clientAddr;
    unsigned int clientAddrSize = sizeof(clientAddr);

    while(isRunning) {
        socketClient = accept(tcpSocket, (struct sockaddr *) &clientAddr, &clientAddrSize);
        co.log("Client connected: " + string(inet_ntoa(clientAddr.sin_addr)) + ":" + to_string((int)clientAddr.sin_port));

        // create thread an detach it so it runs on its own
        cThr = std::thread(&dspManager::clientListen, this, socketClient);
        cThr.detach();
    }
}

void dspManager::generateInitialChannelConfig()
{
    string defaultFreq = "24800000";
    int defaultMasterPort = 40000;
    int masterPortOffset = 100;
    int externalPortOffset = 10000;
    int cI, sI;
    int channels = cfgGlobal.getNumber("channels");
    int slotsPerChannel = cfgGlobal.getNumber("slotsPerChannel");

    cfgChannels = Config("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/DspMapping.cfg");

    for (cI = 0;cI < channels;cI++)
    {
        int masterPort = defaultMasterPort + cI * masterPortOffset;
        cfgChannels.setValue(co.getMasterKey(cI), string(to_string(masterPort) + "," + to_string(masterPort + externalPortOffset) + "," + defaultFreq));

        for(sI = 0;sI < slotsPerChannel;sI++)
        {
            int slavePort = masterPort + 1 + sI;
            cfgChannels.setValue(co.getSlaveKey(cI, sI), string(to_string(slavePort) + "," + to_string(slavePort + externalPortOffset) + "," + defaultFreq));
        }
    }

    cfgChannels.save();
}

void dspManager::clientListen(int socketClient)
{
    string socketStr = to_string(socketClient);
    co.log("Socket(" + socketStr + ") listen");
    while (isRunning){
        bytes_read = read(socketClient, receiveBuffer, sizeof(receiveBuffer));// recvfrom(socketClient, receiveBuffer, sizeof(receiveBuffer), 0, (struct sockaddr *)&socketAddr,&socketAddrSize);

        if (bytes_read < 0)
        {
            co.log("Error while reading from tcp socket.");
            //break;
        }
        else
        {
            std::string st(receiveBuffer);
            if (st.length() <= 0)
                continue;
            else if (st == "disconnect()")
                break;

            string response = handler.handle(st);
            bytes_read = send(socketClient, response.c_str(), response.length() + 1, 0);
            if (bytes_read < 0)
                co.log("error sending response");
            co.log("Socket(" + socketStr + ") -> " + response);
        }
    }
    close(socketClient);
    co.log("Socket(" + socketStr + ") closed");
}
