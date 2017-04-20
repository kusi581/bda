#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>

using namespace std;

class Common
{
public:
    Common();
    void initLog(string component, bool enabled);
    void log(string msg1);
    void log(string msg1, string msg2);

    // string operations
    template<typename Out>
    void split(const std::string &s, char delim, Out result);
    std::vector<std::string> split(const std::string &s, char delim);
    string join(vector<string> &v, string delim);
    string toLower(string text);

    // config keys
    string getMasterKey(int channel);
    string getSlaveKey(int channel, int slave);
    string getMasterKey(string channel);
    string getSlaveKey(string channel, string slave);
    string getChannelKey(string channel);
    string getChannelKey(int channel);
private:
    string component;
    bool enabled;
};

#endif // COMMON_H
