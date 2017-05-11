#include "lifecyclemanager.h"
#include "typedefinitions.h"
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <chrono>

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

    if (threads[slaveNr].joinable())
    {
        co.log("Observer for slave " + to_string(slaveNr) + " is still running");
        return;
    }

    threads[slaveNr] = thread(&lifecycleManager::observeSlaveThread, this, rawDspCommand, slaveNr);
}

void lifecycleManager::observeSlaveThread(string command, int nr)
{
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
}

lifecycleManager::lifecycleManager()
{
    co.initLog("lcy", true);

    cfgSlaves = Config("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/Slaves.cfg");
    cfgSlaves.load();

    Config cfg("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/SignalManager.cfg");
    cfg.load();

    checkInterval = cfg.getNumber("dspServerCheckInterval");
    slaves = cfg.getNumber("slaves");

    // one thread per slave
    threads = vector<thread>(slaves);
}
