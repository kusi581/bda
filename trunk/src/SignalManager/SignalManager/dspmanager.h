#ifndef DSPMANAGER_H
#define DSPMANAGER_H

#include <thread>
#include "common.h"
#include <functional>
#include "config.h"
#include <string>

using namespace std;

class dspManager
{
public:
    dspManager();
    void setupSocket();
    void startListener();
    void stopListener();
    bool isRunning;
private:
    bool setupOk;
    std::thread listenThread;
    void waitForConnection();
    void clientListen(int clientSocket);
    string handleCommand(string raw);
    Config cfgChannels;
    Config cfgGlobal;
    void generateInitialChannelConfig();

    // commands
    string getChannels();
    string startChannel(string argument);
    string getChannelInfo(string argument);
};

#endif // DSPMANAGER_H
