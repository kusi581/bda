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
int nrOfConnections = 5;

dspManager::dspManager()
{
    co.initLog("dsp", true);
    isRunning = false;
    setupOk = false;

    cfgGlobal = Config("./SignalManager.cfg");

    if (!cfgGlobal.exists())
        writeInitialSignalConfig();

    cfgGlobal.load();

    nrOfConnections = cfgGlobal.getNumber("nrOfConnections");

    cfgChannels = Config("./DspMapping.cfg");
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
    //listenThread.join();
}

void dspManager::setupSocket()
{
    string socketIp = cfgGlobal.getValue("localIp");
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
    socketAddr.sin_addr.s_addr = inet_addr(socketIp.c_str());
    socketAddr.sin_port = htons(port);

    if (bind(tcpSocket, (struct sockaddr *) &socketAddr, socketAddrSize) < 0)
    {
        setupOk = false;
        co.log("Tcp socket error: bind()");
        return;
    }

    co.log("listen on " + socketIp + ":" + to_string(port));

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
        bytes_read = read(socketClient, receiveBuffer, sizeof(receiveBuffer));

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

void dspManager::writeInitialSignalConfig()
{
    cfgGlobal.enableSaveOnChange(false);
    cfgGlobal.setValue("channels", "2");
    cfgGlobal.setValue("slaves", "8");
    cfgGlobal.setValue("tcpListenPort", "55555");
    cfgGlobal.setValue("masterPortStart", "50000");
    cfgGlobal.setValue("dspServerCheckInterval", "5000");
    cfgGlobal.setValue("defaultFrequency", "28400000");
    cfgGlobal.setValue("localIp", "127.0.0.1");
    cfgGlobal.setValue("metisIf", "wlp2s0");
    cfgGlobal.setValue("nrOfConnections", "10");
    cfgGlobal.save();
    cfgGlobal.enableSaveOnChange(true);

    co.log("inital master config written");
}
