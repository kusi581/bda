#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include "config.h"
#include <string>
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

    // commands
    bool isValid(string command);
    string getChannels();
    string getChannelInfo(string argument);
    string startChannel(string argument);
    string listenChannel(string argument);
};

#endif // COMMANDHANDLER_H
