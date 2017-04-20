#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include "config.h"
#include <string>
#include <stdio.h>
#include <map>
using namespace std;

class commandHandler
{
public:
    commandHandler();
    string handle(string rawCommand);

private:
    Common co;
    Config cfgChannels;
    Config cfgGlobal;
    typedef string (commandHandler::*cmdPtr)(string);

    // commands
    static bool isInitialized;
    static void initCommandMap();
    static map<string, cmdPtr> commandMap;

    bool isValid(string command);
    string getChannels(string argument);
    string getChannelInfo(string argument);
    string startChannel(string argument);
    string listenChannel(string argument);

    // starting programs
    string getDspCommand(bool isMaster, string dspTcpPort, string receiver, string dspIqPort, string hwIqPort);
    string getWebsocketCommand(string dspWsPort, string dspTcpPort);
    string wrapStartCommand(string command);
    void startMultiplexer(int channel);

    // config
    void writeDspPort(int channel, int slave, string port);
};

#endif // COMMANDHANDLER_H
