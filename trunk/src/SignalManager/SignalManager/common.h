#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <iterator>

using namespace std;

class Common
{
public:
    Common();
    void initLog(string component, bool enabled);
    void log(string msg1);
    void log(string msg1, string msg2);

    // split
    template<typename Out>
    void split(const std::string &s, char delim, Out result);
    std::vector<std::string> split(const std::string &s, char delim);

    // config keys
    string getMasterKey(int channel);
    string getSlaveKey(int channel, int slave);
    string getMasterKey(string channel);
    string getSlaveKey(string channel, string slave);
private:
    string component;
    bool enabled;
};

#endif // COMMON_H
