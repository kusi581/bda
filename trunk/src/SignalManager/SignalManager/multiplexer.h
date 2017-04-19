#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "common.h"
#include <thread>
using namespace  std;

class multiplexer
{
public:
    multiplexer();
    void init(int checkInterval);
    void start();
    void stop();
private:
    Common co;
    int checkInterval;
    bool isRunning;
    thread threads[10];
    void startMultiplexing(int channel);
};

#endif // MULTIPLEXER_H
