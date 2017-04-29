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

    int port = cfgGlobal.getNumber("tcpListenPort");

    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = inet_addr("192.168.1.111");
    socketAddr.sin_port = htons(port);

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
    co.log("setup ok, port " + to_string(port));
}

void dspManager::waitForConnection()
{
    std::thread cThr;
    int socketClient;
    struct sockaddr_in clientAddr;
    unsigned int clientAddrSize = sizeof(clientAddr);

    while(isRunning) {
        co.log("Waiting for client...");
        socketClient = accept(tcpSocket, (struct sockaddr *) &clientAddr, &clientAddrSize);
        co.log("Client connected: " + string(inet_ntoa(clientAddr.sin_addr)) + ":" + to_string((int)clientAddr.sin_port));

        // create thread an detach it so it runs on its own
        cThr = std::thread(&dspManager::clientListen, this, socketClient);
        cThr.detach();
    }
}

void dspManager::clientListen(int socketClient)
{
    string socketStr = "Socket(" + to_string(socketClient) + "):";
    co.log(socketStr + " start listen");
    while (isRunning){
        co.log(socketStr + " wait for command");
        bytes_read = read(socketClient, receiveBuffer, sizeof(receiveBuffer));// recvfrom(socketClient, receiveBuffer, sizeof(receiveBuffer), 0, (struct sockaddr *)&socketAddr,&socketAddrSize);

        if (bytes_read < 0)
        {
            co.log(socketStr + " read error");
            //break;
        }
        else
        {
            std::string st(receiveBuffer);
            if (st.length() <= 0)
                continue;
            else if (st == "disconnect()")
            {
                co.log(socketStr + " disconnected");
                isRunning = false;
            }
            else
            {
                handler = new commandHandler();
                string response = handler->handle(st);
                co.log(socketStr + " " + st + " -> " + response);
                bytes_read = send(socketClient, response.c_str(), response.length() + 1, 0);
                if (bytes_read < 0)
                    co.log(socketStr + " send response error");
            }
        }

        break; // only handle one command per connection
    }
    close(socketClient);
    co.log(socketStr + " closed");
}
