#ifndef LIFECYCLEMANAGER_H
#define LIFECYCLEMANAGER_H

#include "common.h"
#include "config.h"
#include <vector>
#include <thread>

using namespace std;

class lifecycleManager
{
public:
    static lifecycleManager *Instance();

    void observeSlave(string rawDspCommand, int slaveNr);
private:
    Common co;
    Config cfgSlaves;

    void observeSlaveThread(string command, int nr);

    static lifecycleManager* m_pInstance;
    lifecycleManager();
    vector<thread> threads;
    int checkInterval;
    int slaves;
};

#endif // LIFECYCLEMANAGER_H
