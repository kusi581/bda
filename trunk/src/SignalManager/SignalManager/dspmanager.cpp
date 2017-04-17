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
Config cfgGlobal;
Config cfgChannels;
int nrOfConnections = 5; // todo: configurable in global
static Common co;

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

// todo: evtl. auslagern in anderes/mehrere file(s)?
string dspManager::handleCommand(string raw)
{
    string result = "f;invalid command";

    // check for valid command
    if (raw.find('(') == string::npos || raw.find(')') == string::npos)
        return result;

    // todo: convert to lower
    string cmd = raw.substr(0, raw.find('('));
    string argument = (raw.find('(') == raw.find(')') - 1) ? "" : raw.substr(raw.find('(') + 1, raw.find(')') - raw.find('(') - 1);

    if (cmd == "getChannels")
        result = getChannels();
    else if (cmd == "startChannel")
        result = startChannel(argument);
    else if (cmd == "getChannelInfo")
        result = getChannelInfo(argument);

    return result;
}

string dspManager::getChannels()
{
    int i = 0;
    string response = "s;";

    cfgChannels.load();

    int channels = cfgGlobal.getNumber("channels");

    while (i < channels)
    {
        response = response + to_string(i);
        if (i < channels - 1)
            response += ";";
        i++;
    }

    return response;
}

string dspManager::getChannelInfo(string argument)
{
    string response;
    int channel = stoi(argument);

    cfgChannels.load();

    string key = co.getMasterKey(argument);
    if (channel >= cfgGlobal.getNumber("channels") || !cfgChannels.keyExists(key))
    {
        response = "f;channel " + argument + " does not exist";
    }
    else
    {
        // todo:
        response = "s;NotRunning";
    }
    return response;
}

string dspManager::startChannel(string argument)
{
    string response;
    string channelKey = co.getMasterKey(argument);

    cfgChannels.load();
    if (!cfgChannels.keyExists(channelKey))
    {
        response = "f;invalid channel";
    }
    else
    {
        // starting dsp
        string dspTcpPort = cfgChannels.getValue(channelKey, 0);

        string dspFile = "/home/kusi/School/bda/repo/trunk/src/dspserver/dspserver";
        string dspCommand = dspFile + " --address 127.0.0.1 --hpsdr --clientport " + dspTcpPort + " --receiver 0";
        // todo: receiver when multiple mods

        string fullCommand = "gnome-terminal -e '" + dspCommand + "'";
        system(fullCommand.c_str());

        // starting websocket bridge
        string dspWsPort = cfgChannels.getValue(channelKey, 1);
        string websockify = "websockify 127.0.0.1:" + dspWsPort + " 127.0.0.1:" + dspTcpPort;

        fullCommand = "gnome-terminal -e '" + websockify + "'";
        system(fullCommand.c_str());

        response = "s;dsp started;" + dspWsPort;
    }

    return response;
}

string setFreq(){

}
