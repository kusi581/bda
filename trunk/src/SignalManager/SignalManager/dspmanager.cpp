#include "dspmanager.h"
#include "common.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include "config.h"
#include <unistd.h>

int tcpSocket;
struct sockaddr_in socketAddr;
char receiveBuffer[512];
int bytes_read;
unsigned int socketAddrSize = sizeof(socketAddr);
Config cfg;
int nrOfConnections = 5;
static Common co;

dspManager::dspManager()
{
    co.initLog("dsp", true);
    isRunning = false;
    setupOk = false;
    cfg = Config("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/SignalManager.cfg");
    cfg.load();
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
   //listenThread.join();

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
    socketAddr.sin_port = htons(cfg.getNumber("tcpListenPort"));

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
    int curr = 0;
    int socketClient;
    struct sockaddr_in clientAddr;
    unsigned int clientAddrSize = sizeof(clientAddr);
    co.log("accept connections...");

    while(isRunning) {
        socketClient = accept(tcpSocket, (struct sockaddr *) &clientAddr, &clientAddrSize);
        co.log("New connection from: ", inet_ntoa(clientAddr.sin_addr));

        clientListen(socketClient);

        //std::thread cThr(&dspManager::clientListen, this, socketClient);
    }
}

void dspManager::clientListen(int socketClient)
{
    co.log("wait for messages...");
    while (isRunning){
        bytes_read = read(socketClient, receiveBuffer, sizeof(receiveBuffer));// recvfrom(socketClient, receiveBuffer, sizeof(receiveBuffer), 0, (struct sockaddr *)&socketAddr,&socketAddrSize);

        if (bytes_read < 0)
        {
            //co.log("Error while reading from tcp socket.");
            //break;
        }
        else
        {
            std::string st(receiveBuffer);
            if (st.length() <= 0)
                continue;

            string response = handleCommand(st);
            bytes_read = send(socketClient, response.c_str(), response.length() + 1, 0);
            if (bytes_read < 0)
                co.log("error sending response");
            co.log("response out: ", response);
        }
    }
    co.log("wait messages stopped");
}

string dspManager::handleCommand(string raw)
{
    string result = "f;invalid command";

    // check for valid command
    if (raw.find("(") == string::npos || raw.find(")") == string::npos)
        return result;

    // todo: convert to lower
    string cmd = raw.substr(0, raw.find("("));

    if (cmd == "getChannels")
        result = getChannels();

    return result;
}

string dspManager::getChannels()
{
    Config channelCfg("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/DspMapping.cfg");
    channelCfg.load();

    // todo: get active channels from config file

    return "f;no channels started";
}
