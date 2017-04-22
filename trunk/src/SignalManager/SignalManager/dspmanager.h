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
    bool setupOk;
    Common co;
    commandHandler *handler;
    Config cfgChannels;
    Config cfgGlobal;

    // thread to listen for new connections
    std::thread listenThread;
    void waitForConnection();

    /**
     * @brief this is called for each new connection to communicate
     * @param clientSocket
     */
    void clientListen(int clientSocket);

    /**
     * @brief generates a simple channel config
     */
    void generateInitialChannelConfig();
};

#endif // DSPMANAGER_H
