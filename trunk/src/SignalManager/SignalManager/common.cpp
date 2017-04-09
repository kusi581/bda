#include "common.h"
#include <iostream>
#include <string>
#include <stdarg.h>
using namespace std;

void Common::log(char *msg)
{
    cout << "[LOG] " << msg << endl;
}

void Common::log(string msg)
{
    cout << "[LOG] " << msg << endl;
}
