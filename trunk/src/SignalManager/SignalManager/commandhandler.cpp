#include "commandhandler.h"

typedef string (commandHandler::*cmdPtr)(string);
bool commandHandler::isInitialized = false;
map<string, cmdPtr> commandHandler::commandMap;

// this class handles the commands sent from the clients
// for each command a new instance of this class is created
commandHandler::commandHandler()
{
    co.initLog("CoH", true);

    if (!commandHandler::isInitialized)
    {
        commandHandler::isInitialized = true;
        commandHandler::initCommandMap();
    }

    cfgGlobal = Config("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/SignalManager.cfg");
    cfgChannels = Config("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/DspMapping.cfg");

    cfgGlobal.load();
    cfgChannels.load();
}

string commandHandler::handle(string raw)
{
    string result = "f;invalid command";
    if (!commandHandler::isValid(raw))
        return result;

    string cmd = co.toLower(raw.substr(0, raw.find('(')));
    string argument = (raw.find('(') == raw.find(')') - 1) ? "" : raw.substr(raw.find('(') + 1, raw.find(')') - raw.find('(') - 1);

    cmdPtr command = commandHandler::commandMap[cmd];

    if (command != NULL)
    {
        co.log("Handling " + raw);
        result = (this->*command)(argument);
    }

    return result;
}

void commandHandler::initCommandMap()
{
    commandHandler::commandMap["getchannels"] = &commandHandler::getChannels;
    commandHandler::commandMap["getchannelinfo"] = &commandHandler::getChannelInfo;
    commandHandler::commandMap["startchannel"] = &commandHandler::startChannel;
    commandHandler::commandMap["listenchannel"] = &commandHandler::listenChannel;
}

bool commandHandler::isValid(string command)
{
    return !(command.find('(') == string::npos || command.find(')') == string::npos);
}

string commandHandler::getChannels(string argument)
{
    int i = 0;
    string response = "s;";

    cfgChannels.load();

    int channels = cfgGlobal.getNumber("channels");

    while (i < channels)
    {
        response = response + to_string(i) + (i < channels - 1 ? ";" : "");
        i++;
    }

    return response;
}

string commandHandler::getChannelInfo(string argument)
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

string commandHandler::startChannel(string argument)
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
        // todo: get correct values from config files
        string dspTcpPort = cfgChannels.getValue(channelKey, 0);
        string hwIqPort = "17000";
        string dspIqPort = "17001";
        string dspWsPort = cfgChannels.getValue(channelKey, 1);

        string command = getDspCommand(true, dspTcpPort, argument, dspIqPort, hwIqPort);
        system(command.c_str());

        command = getWebsocketCommand(dspWsPort, dspTcpPort);
        system(command.c_str());

        response = "s;dsp started;" + dspWsPort;
    }

    return response;
}

string commandHandler::listenChannel(string argument)
{
    string response;
    string channelKey = co.getSlaveKey(argument, "0"); // todo: choose correct

    cfgChannels.load();
    if (!cfgChannels.keyExists(channelKey))
    {
        response = "f;invalid channel";
    }
    else
    {
        // todo: get correct values from config files
        string dspTcpPort = cfgChannels.getValue(channelKey, 0);
        string dspIqPort = "17002";
        string dspWsPort = cfgChannels.getValue(channelKey, 1);

        string command = getDspCommand(false, dspTcpPort, argument, dspIqPort, "");
        system(command.c_str());

        command = getWebsocketCommand(dspWsPort, dspTcpPort);
        system(command.c_str());

        response = "s;dsp started;" + dspWsPort;
    }
    return response;
}

string commandHandler::getDspCommand(bool isMaster, string dspTcpPort, string receiver, string dspIqPort, string hwIqPort)
{
    string command = "/home/kusi/School/bda/repo/trunk/src/dspserver/dspserver";

    command += " --address 127.0.0.1";
    command += " --hpsdr";
    command += " --clientport " + dspTcpPort;
    command += " --receiver " + receiver;
    command += " --dspiqport " + dspIqPort;

    if (isMaster)
    {
        command += " --master";
        command += " --hwiqport " + hwIqPort;
    }

    return wrapStartCommand(command);
}

string commandHandler::getWebsocketCommand(string dspWsPort, string dspTcpPort)
{
    return wrapStartCommand("websockify 127.0.0.1:" + dspWsPort + " 127.0.0.1:" + dspTcpPort);
}

string commandHandler::wrapStartCommand(string command)
{
    return "gnome-terminal -e '" + command + "'";
}

void commandHandler::writeDspPort(int channel, int part, string port)
{
    Config cfg("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/Multiplexer.cfg");
    cfg.load();

    cfg.setValue(co.getChannelKey(channel), port, part);
}
