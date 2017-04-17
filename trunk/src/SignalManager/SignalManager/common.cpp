#include "common.h"
#include <iostream>
#include <string>
#include <stdarg.h>
#include <sys/time.h>
#include <sstream>
#include <vector>
#include <iterator>
using namespace std;

Common::Common()
{
    initLog("com", false);
}

void Common::initLog(string component, bool enabled)
{
    this->component = component;
    this->enabled = enabled;
}

void Common::log(string msg, string msg2) {
    if (!this->enabled)
        return;

    struct timeval tp;
    gettimeofday(&tp, NULL);

    time_t t = time(0);
    struct tm* now = localtime(&t);

    cout << "["
         << (now->tm_year +1900) << '-'
         << now->tm_mon << '-'
         << now->tm_mday << '_'
         << now->tm_hour << ':'
         << now->tm_min << ':'
         << now->tm_sec << '.'
         << (tp.tv_usec)
         << "|" << component << "] " << msg << msg2 << endl;
}

void Common::log(string msg)
{
    if (!this->enabled)
        return;
    struct timeval tp;
    gettimeofday(&tp, NULL);

    time_t t = time(0);
    struct tm* now = localtime(&t);

    cout << "["
         << (now->tm_year +1900) << '-'
         << now->tm_mon << '-'
         << now->tm_mday << '_'
         << now->tm_hour << ':'
         << now->tm_min << ':'
         << now->tm_sec << '.'
         << (tp.tv_usec)
         << "|" << component << "] " << msg << endl;
}

template<typename Out>
void Common::split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> Common::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

string Common::getMasterKey(int channel)
{
    return getMasterKey(to_string(channel));
}

string Common::getSlaveKey(int channel, int slave)
{
    return getSlaveKey(to_string(channel), to_string(slave));
}

string Common::getMasterKey(string channel)
{
    return string("ch" + channel + "ma");
}

string Common::getSlaveKey(string channel, string slave)
{
    return string("ch" + channel + "s" + slave);
}
