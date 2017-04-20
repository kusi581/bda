#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "common.h"
#include "config.h"
#include <thread>
#include <map>
#include <vector>
using namespace  std;

class multiplexer
{
public:
    multiplexer();
    void init();
    void start(int channel);
    void stop();
private:
    int checkInterval;
    int channels;
    bool isRunning;

    Common co;
    vector<thread> threads;

    map<int, vector<int>> ports;

    void startMultiplexing(int channel);
    void loadPorts();
};

#endif // MULTIPLEXER_H
