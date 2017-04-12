#ifndef DSPMANAGER_H
#define DSPMANAGER_H

#include <thread>
#include "common.h"

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

    // command handler
    string getChannels();
};

#endif // DSPMANAGER_H
