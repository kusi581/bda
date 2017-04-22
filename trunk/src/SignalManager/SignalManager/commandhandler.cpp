#include "commandhandler.h"
#include "commandresponse.h"
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

typedef string (commandHandler::*cmdPtr)(string);
bool commandHandler::isInitialized = false;
map<string, cmdPtr> commandHandler::commandMap;

commandHandler::commandHandler()
{
    co.initLog("CoH", true);

    // init of command map only once
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
    string result = "";
    if (!commandHandler::isValid(raw))
    {
        response.setInvalidCommand();
        return response.getJson();
    }

    string cmd = co.toLower(raw.substr(0, raw.find('(')));
    string argument = (raw.find('(') == raw.find(')') - 1) ? "" : raw.substr(raw.find('(') + 1, raw.find(')') - raw.find('(') - 1);

    cmdPtr command = commandHandler::commandMap[cmd];

    // magic pointer to member callback
    if (command != NULL)
        result = (this->*command)(argument);

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
    cfgChannels.load();

    string channels = cfgGlobal.getValue("channels");

    response.setState(true);
    response.set("channels", channels);
    return response.getJson();
}

string commandHandler::getChannelInfo(string argument)
{
    int channel = stoi(argument);

    cfgChannels.load();

    string key = co.getMasterKey(argument);
    if (channel >= cfgGlobal.getNumber("channels") || !cfgChannels.keyExists(key))
    {
        response.setState(false);
        response.setMessage("invalid channel: " + argument);
    }
    else
    {
        // todo:
        response.setState(true);
        response.set("info", "todo...");
    }
    return response.getJson();
}

string commandHandler::startChannel(string argument)
{
    string channelKey = co.getMasterKey(argument);

    cfgChannels.load();
    if (!cfgChannels.keyExists(channelKey))
    {
        response.setState(false);
        response.setMessage("invalid channel: " + argument);
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

        response.setState(true);
        response.setMessage("dsp started");
        response.set("port", dspWsPort);
    }

    return response.getJson();
}

string commandHandler::listenChannel(string argument)
{
    string channelKey = co.getSlaveKey(argument, argument);

    cfgChannels.load();
    if (!cfgChannels.keyExists(channelKey))
    {
        response.setState(false);
        response.setMessage("invalid channel: " + argument);
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

        response.setState(true);
        response.setMessage("dsp started");
        response.set("port", dspWsPort);
    }
    return response.getJson();
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
