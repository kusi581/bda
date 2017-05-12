#include "lifecyclemanager.h"
#include "typedefinitions.h"
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
    if (threadStates[slaveNr] == ObserverState::Observing)
    {
        co.log("Observer for slave " + to_string(slaveNr) + " is still running");
        return;
    }

    if (threadStates[slaveNr] == ObserverState::Finished)
    {
        co.log("Observer for slave " + to_string(slaveNr) + " has finished");
        threads[slaveNr].join();
    }

    threads[slaveNr] = thread(&lifecycleManager::observeSlaveThread, this, rawDspCommand, slaveNr);
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

void lifecycleManager::observeSlaveThread(string command, int nr)
{
    threadStates[nr] = ObserverState::Observing;
    int bufLen = 128, pid = -1;
    char path[bufLen];
    string pgrepCmd = "pgrep -o -f '" + command + "'";
    string sKey = "s" + to_string(nr);

    FILE *fp = popen(pgrepCmd.c_str(), "r");
    while (fgets(path, bufLen, fp) != NULL)
        pid = stoi(path);

    pclose(fp);

    if (pid <= 0)
    {
        co.log("DspServer not running for slave " + to_string(nr));
        return;
    }

    while (kill(pid, 0) == 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(checkInterval));

    co.log("DspServer stopped, slave " + to_string(nr));

    cfgSlaves.setValue(sKey, to_string(NotRunning), 0);
    cfgSlaves.setValue(sKey, to_string(-1), 2);

    threadStates[nr] = ObserverState::Finished;
}

lifecycleManager::lifecycleManager()
{
    co.initLog("lcy", true);

    cfgSlaves = Config("./Slaves.cfg");
    cfgSlaves.load();

    Config cfg("./SignalManager.cfg");
    cfg.load();

    checkInterval = cfg.getNumber("dspServerCheckInterval");
    slaves = cfg.getNumber("slaves");

    // one thread per slave
    threads = vector<thread>(slaves);
    threadStates = vector<State>(slaves);
}
