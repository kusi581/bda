#ifndef LIFECYCLEMANAGER_H
#define LIFECYCLEMANAGER_H

#include "common.h"
#include "typedefinitions.h"
#include "config.h"
#include <vector>
#include <thread>
#include <future>

using namespace std;
using namespace ObserverState;

class lifecycleManager
{
public:
    static lifecycleManager *Instance();

    void observeSlave(string rawDspCommand, int slaveNr);

    bool isRunning(string rawDspCommand);
private:
    Common co;
    Config cfgSlaves;

    void observeSlaveThread(string command, int nr);

    static lifecycleManager* m_pInstance;
    lifecycleManager();
    vector<thread> threads;
    vector<State> threadStates;
    int checkInterval;
    int slaves;
};

#endif // LIFECYCLEMANAGER_H
