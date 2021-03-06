#include "commandhandler.h"
#include "commandresponse.h"
#include "multiplexer.h"
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#include "typedefinitions.h"

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

    cfgGlobal = Config("./SignalManager.cfg");
    cfgChannels = Config("./Channels.cfg");
    cfgSlaves = Config("./Slaves.cfg");

    cfgGlobal.load();
    cfgChannels.load();
    cfgSlaves.load();
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
    commandHandler::commandMap["init"] = &commandHandler::init;
    commandHandler::commandMap["getchannels"] = &commandHandler::getChannels;
    commandHandler::commandMap["startchannel"] = &commandHandler::startChannel;
    commandHandler::commandMap["listenchannel"] = &commandHandler::listenChannel;
}

bool commandHandler::isValid(string command)
{
    return !(command.find('(') == string::npos || command.find(')') == string::npos);
}

string commandHandler::init(string argument)
{
    int rate = argument.length() == 0 ? 48000 : stoi(argument);

    if (rate == 48000 || rate == 96000 || rate == 192000)
    {
        writeInitialChannelConfig(cfgGlobal.getNumber("channels"));
        writeInitialSlaveConfig(cfgGlobal.getNumber("slaves"));

        string command = getHwServerCommand(argument);
        system(command.c_str());

        response.setState(true);
        response.setMessage("init complete");
    }
    else
    {
        response.setState(false);
        response.setMessage("invalid samplingrate: " + argument);
    }
    return response.getJson();
}

string commandHandler::getChannels(string argument)
{
    int channels = cfgGlobal.getNumber("channels");

    response.initArray(channels);
    for (int i = 0; i < channels;i++)
    {
        string cKey = "ch" + to_string(i);

        response.addArrayEntry(i, "freq", cfgChannels.getValue(cKey, 0));
        response.addArrayEntry(i, "state", (cfgChannels.getNumber(cKey, 1) != InUse ? "free" : "notFree"));
    }
    return response.getJson();
}

string commandHandler::startChannel(string argument)
{
    int channel = stoi(argument);
    int dspPortRoot = cfgGlobal.getNumber("masterPortStart") + (channel * 10);
    string cKey = "ch" + argument;

    if (!cfgChannels.keyExists(cKey))
    {
        response.setState(false);
        response.setMessage("invalid channel: " + argument);
    }
    else if(cfgChannels.getNumber(cKey,1) == InUse)
    {
        response.setState(false);
        response.setMessage("channel is in use: " + argument);
    }
    else if (channel >= 0 && channel < cfgGlobal.getNumber("channels"))
    {
        string dspTcpPort   = to_string(dspPortRoot + 1);
        string hwIqPort     = to_string(dspPortRoot + 2);
        string dspIqPort    = to_string(dspPortRoot + 3);
        string dspWsPort    = to_string(dspPortRoot);

        response.setState(true);
        if (cfgChannels.getNumber(cKey, 1) == NotRunning)
        {
            // DspTCP               = masterportstart + (channel * 10) + 1
            // multiplexerIqPort    = masterportstart + (channel * 10) + 2
            // dspIqPort            = masterportstart + (channel * 10) + 3
            // dspWsPort            = masterportstart + (channel * 10)

            string command = getDspCommand(true, dspTcpPort, argument, dspIqPort, hwIqPort);
            string dspRawCommand = command;

            if (!lifecycleManager::Instance()->isRunning(command))
            {
                //co.log("Start master cmd: " + command);
                command = wrapStartCommand(command);
                system(command.c_str());
            }

            command = getWebsocketCommand(dspWsPort, dspTcpPort);
            system(command.c_str());

            response.setMessage("dsp started");
            cfgChannels.setValue(cKey, to_string(Running), 1);
            cfgChannels.save();

            // notify multiplexer, so it reloads the configuration
            multiplexer::Instance()->start(channel);

            // start observer
            lifecycleManager::Instance()->observeMaster(dspRawCommand, channel);
        }
        else
        {
            response.setMessage("dsp already started");
        }
        response.set("port", dspWsPort);
    }
    else
    {
        response.setState(false);
        response.setMessage("invalid channel: " + argument);
    }

    return response.getJson();
}

string commandHandler::listenChannel(string argument)
{
    int channels = cfgGlobal.getNumber("channels");
    int channel = stoi(argument);
    int slave = getNextAvailableSlave();

    if (slave < 0)
    {
        response.setState(false);
        response.setMessage("no more slaves available");
    }
    else if (channel >= 0 && channel < channels)
    {
        string sKey = "s" + to_string(slave);
        int dspPortRoot = cfgGlobal.getNumber("masterPortStart") + (channels * 10) + (slave * 5);
        string dspTcpPort   = to_string(dspPortRoot + 1);
        string dspIqPort    = to_string(dspPortRoot + 3);
        string dspWsPort    = to_string(dspPortRoot);

        string command = getDspCommand(false, dspTcpPort, argument, dspIqPort, "");
        string dspRawCommand = command;

        if (!lifecycleManager::Instance()->isRunning(dspRawCommand))
        {
            command = wrapStartCommand(command);
            system(command.c_str());
        }

        command = getWebsocketCommand(dspWsPort, dspTcpPort);
        system(command.c_str());

        response.setState(true);
        response.setMessage("dsp started");
        response.set("port", dspWsPort);

        cfgSlaves.setValue(sKey, to_string(Running), 0);
        cfgSlaves.setValue(sKey, to_string(slave), 2);               

        // notify multiplexer, so it reloads the configuration
        multiplexer::Instance()->start(channel);

        // start observer
        lifecycleManager::Instance()->observeSlave(dspRawCommand, slave);
    }
    else
    {
        response.setState(false);
        response.setMessage("invalid channel: " + argument);
    }
    return response.getJson();
}

string commandHandler::getDspCommand(bool isMaster, string dspTcpPort, string receiver, string dspIqPort, string hwIqPort)
{
    string command = "/home/kusi/School/bda/repo/trunk/src/dspserver/dspserver";

    command += " --address " + cfgGlobal.getValue("localIp");
    command += " --hpsdr";
    command += " --clientport " + dspTcpPort;
    command += " --receiver " + receiver;
    command += " --dspiqport " + dspIqPort;

    if (isMaster)
    {
        command += " --master";
        command += " --hwiqport " + hwIqPort;
    }

    return command;
}

string commandHandler::getHwServerCommand(string samplingrate)
{
    string command = "/home/kusi/School/bda/repo/trunk/src/server/hpsdr-server";

    command += " --metis";
    command += " --receiver " + (cfgGlobal.getNumber("channels") > 4 ? "4" : cfgGlobal.getValue("channels"));
    command += " --interface " + cfgGlobal.getValue("metisIf");
    command += " --samplerate " + samplingrate;

    return wrapStartCommand(command);
}

string commandHandler::getWebsocketCommand(string dspWsPort, string dspTcpPort)
{
    string localIp = cfgGlobal.getValue("localIp");
    return wrapStartCommand("websockify " + localIp + ":" + dspWsPort + " " + localIp + ":" + dspTcpPort);
}

string commandHandler::wrapStartCommand(string command)
{
    return "gnome-terminal -e '" + command + "'";
}

void commandHandler::writeDspPort(int channel, int part, string port)
{
    Config cfg("./Multiplexer.cfg");
    cfg.load();

    cfg.setValue(co.getChannelKey(channel), port, part);
}

void commandHandler::writeInitialChannelConfig(int channels)
{
    int masterPort = cfgGlobal.getNumber("masterPortStart");
    string defFreq = cfgGlobal.getValue("defaultFrequency");
    Config cfg("./Channels.cfg");
    string channelKey;
    cfg.enableSaveOnChange(false);
    for(int i = 0;i<channels;i++)
    {
        int dspPortRoot = masterPort + (i * 10);

        channelKey = "ch" + to_string(i);
        cfg.setValue(channelKey, "FREQ,STATE,MulIQPort,DspIQPOrt");
        cfg.setValue(channelKey, defFreq, 0);
        cfg.setValue(channelKey, to_string(NotRunning), 1);
        cfg.setValue(channelKey, to_string(dspPortRoot + 2), 2);
        cfg.setValue(channelKey, to_string(dspPortRoot + 3), 3);
    }
    cfg.enableSaveOnChange(true);
    cfg.save();
}

void commandHandler::writeInitialSlaveConfig(int slaves)
{
    int masterPort = cfgGlobal.getNumber("masterPortStart");
    int channels = cfgGlobal.getNumber("channels");
    Config cfg("./Slaves.cfg");
    string channelKey;
    cfg.enableSaveOnChange(false);
    for(int i = 0;i<slaves;i++)
    {
        int dspPortRoot = masterPort + (channels * 10) + (i * 5);

        channelKey = "s" + to_string(i);
        cfg.setValue(channelKey, "STATE,DspIQPOrt,ChannelNr");
        cfg.setValue(channelKey, to_string(NotRunning), 0);
        cfg.setValue(channelKey, to_string(dspPortRoot + 3), 1);
        cfg.setValue(channelKey, to_string(-1), 2);
    }
    cfg.enableSaveOnChange(true);
    cfg.save();
}

int commandHandler::getNextAvailableSlave()
{
    int slave = -1;
    int slaves = cfgGlobal.getNumber("slaves");
    for (int i = 0; i < slaves; i++)
    {
        string slaveKey = "s" + to_string(i);
        slave = cfgSlaves.getNumber(slaveKey, 0) == 0 ? i : -1;
        if (slave >= 0)
            break;
    }
    return slave;
}
