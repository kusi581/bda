#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <stdio.h>
using namespace std;

class Common
{
public:
    Common();
    void initLog(string component, bool enabled);
    void log(string msg1);
    void log(string msg1, string msg2);
private:
    string component;
    bool enabled;
};

#endif // COMMON_H
