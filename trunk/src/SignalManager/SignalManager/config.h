#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

class Config
{
public:
    Config();
    void loadFrom(string filename);
    void saveTo(string filename);

    int tcpListenPort;
    int nrOfModerators;
    int nrOfSlavesPerModerator;
private:
    fstream cfg;
    void parseLine(string line);
};

#endif // CONFIG_H
