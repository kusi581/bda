#ifndef DSPMANAGER_H
#define DSPMANAGER_H

#include "common.h"
#include "config.h"
#include "commandhandler.h"
#include <thread>
#include <functional>
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
    Common co;
    commandHandler handler;
    bool setupOk;
    std::thread listenThread;
    void waitForConnection();
    void clientListen(int clientSocket);
    string handleCommand(string raw);
    Config cfgChannels;
    Config cfgGlobal;
    void generateInitialChannelConfig();
};

#endif // DSPMANAGER_H
