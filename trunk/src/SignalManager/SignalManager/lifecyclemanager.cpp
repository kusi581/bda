#include "lifecyclemanager.h"
#include "typedefinitions.h"
#include "multiplexer.h"
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <chrono>

#include <string>
#include <limits.h>
#include <unistd.h>

lifecycleManager *lifecycleManager::m_pInstance = NULL;

lifecycleManager *lifecycleManager::Instance()
{
    if (!m_pInstance)
        m_pInstance = new lifecycleManager();

    return m_pInstance;
}

void lifecycleManager::observeSlave(string rawDspCommand, int slaveNr)
{
    if (slaveNr < 0 || slaveNr > slaves - 1)
    {
        co.log("Invalid slave specified");
        return;
    }

    observeDsp(rawDspCommand, slaveNr + channels);
}

void lifecycleManager::observeMaster(string rawDspCommand, int channelNr)
{
    if (channelNr < 0 || channelNr > channels - 1)
    {
        co.log("Invalid master specified");
        return;
    }

    observeDsp(rawDspCommand, channelNr);
}

void lifecycleManager::observeDsp(string rawDspCommand, int nr)
{
    if (threadStates[nr] == ObserverState::Observing)
    {
        co.log("Observer for dsp server " + to_string(nr) + " is still running");
        return;
    }

    if (threadStates[nr] == ObserverState::Finished)
    {
        co.log("Observer for dsp server " + to_string(nr) + " has finished");
        threads[nr].join();
    }

    threads[nr] = thread(&lifecycleManager::observeDspThread, this, rawDspCommand, nr);
}

bool lifecycleManager::isRunning(string rawDspCommand)
{
    int bufLen = 128, nr = 0;
    char path[bufLen];
    string pgrepCmd = "pgrep -c -f '" + rawDspCommand + "'";

    FILE *fp = popen(pgrepCmd.c_str(), "r");
    while (fgets(path, bufLen, fp) != NULL)
    {
        nr = stoi(path);
    }

    pclose(fp);

    return nr > 1;
}

void lifecycleManager::observeDspThread(string command, int nr)
{
    threadStates[nr] = ObserverState::Observing;
    int bufLen = 128, pid = -1;
    char path[bufLen];
    string pgrepCmd = "pgrep -o -f '" + command + "'";

    FILE *fp = popen(pgrepCmd.c_str(), "r");
    while (fgets(path, bufLen, fp) != NULL)
        pid = stoi(path);

    pclose(fp);

    if (pid <= 0)
    {
        co.log("DspServer " + to_string(nr) + " not running");
        return;
    }

    while (kill(pid, 0) == 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(checkInterval));

    co.log("DspServer " + to_string(nr) + " stopped");

    // master or slave
    if (nr >= channels)
    {
        string sKey = "s" + to_string(nr - channels);
        cfgSlaves.setValue(sKey, to_string(NotRunning), 0);
        cfgSlaves.setValue(sKey, to_string(-1), 2);
    }
    else
    {
        string cKey = "ch" + to_string(nr);
        cfgChannels.setValue(cKey, to_string(NotRunning), 1);
    }

    threadStates[nr] = ObserverState::Finished;

    // reload multiplexer
    multiplexer::Instance()->loadPorts();
}

lifecycleManager::lifecycleManager()
{
    co.initLog("lcy", true);

    cfgSlaves = Config("./Slaves.cfg");
    cfgSlaves.load();

    cfgChannels = Config("./Channels.cfg");
    cfgChannels.load();

    Config cfg("./SignalManager.cfg");
    cfg.load();

    checkInterval = cfg.getNumber("dspServerCheckInterval");
    slaves = cfg.getNumber("slaves");
    channels = cfg.getNumber("channels");

    // one thread per dsp server, master and slaves combined
    threads = vector<thread>(slaves + channels);
    threadStates = vector<State>(slaves + channels);
}
