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

    /**
     * @brief start starts a new thread for a multiplexer of a given channel
     * @param channel
     */
    void start(int channel);

    void stop();
private:
    int checkInterval;
    int channels;
    bool isRunning;
    Common co;

    /**
     * @brief threads list of all multiplexer threads
     */
    vector<thread> threads;

    /**
     * @brief ports mapping from the multiplex receive port to the dsp server ports
     */
    static map<int, vector<int>> ports;
    static bool portLock;

    /**
     * @brief startMultiplexing starts
     * @param channel
     */
    void startMultiplexing(int channel);

    /**
     * @brief loadPorts loads the multiplex ports from a configuration file,
     * this is calles periodically while multiplexing, to ensure new dsp servers
     * receive the packets too
     */
    void loadPorts();
};

#endif // MULTIPLEXER_H
