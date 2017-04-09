#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <stdio.h>
using namespace std;

class Common
{
public:
    static void log(char* msg);
    static void log(string msg);
private:
    Common();
};

#endif // COMMON_H
