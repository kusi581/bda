#include "common.h"
#include <iostream>
#include <string>
#include <stdarg.h>
#include <sys/time.h>
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
