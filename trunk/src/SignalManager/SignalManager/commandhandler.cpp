#include "commandhandler.h"

// this class handles the commands sent from the clients
// for each command a new instance of this class is created
commandHandler::commandHandler()
{
    co.initLog("CoH", true);

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

    if (cmd == "getchannels")
        result = getChannels();
    else if (cmd == "startchannel")
        result = startChannel(argument);
    else if (cmd == "listenchannel")
        result = listenChannel(argument);
    else if (cmd == "getchannelinfo")
        result = getChannelInfo(argument);

    return result;
}

bool commandHandler::isValid(string command)
{
    return !(command.find('(') == string::npos || command.find(')') == string::npos);
}

string commandHandler::getChannels()
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
        // starting dsp
        string dspTcpPort = cfgChannels.getValue(channelKey, 0);

        string dspFile = "/home/kusi/School/bda/repo/trunk/src/dspserver/dspserver";
        string dspCommand = dspFile + " --address 127.0.0.1 --hpsdr --master --clientport " + dspTcpPort + " --receiver " + argument + " --hwiqport 12222 --dspiqport 12223";
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
        // starting dsp
        string dspTcpPort = cfgChannels.getValue(channelKey, 0);

        string dspFile = "/home/kusi/School/bda/repo/trunk/src/dspserver/dspserver";
        string dspCommand = dspFile + " --address 127.0.0.1 --hpsdr --clientport " + dspTcpPort + " --receiver " + argument + " --dspiqport 12224";

        co.log("START DSP: " + dspCommand);

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
