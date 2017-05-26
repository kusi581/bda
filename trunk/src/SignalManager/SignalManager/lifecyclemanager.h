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
    void observeMaster(string rawDspCommand, int channelNr);

    bool isRunning(string rawDspCommand);
private:
    Common co;
    Config cfgSlaves;
    Config cfgChannels;

    void observeDsp(string rawDspCommand, int nr);
    void observeDspThread(string rawDspCommand, int nr);

    static lifecycleManager* m_pInstance;
    lifecycleManager();
    vector<thread> threads;
    vector<State> threadStates;
    int checkInterval;
    int channels;
    int slaves;
};

#endif // LIFECYCLEMANAGER_H
