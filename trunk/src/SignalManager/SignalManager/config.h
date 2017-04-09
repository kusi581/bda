#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <map>
using namespace std;

enum Setting {
    ConfigVersion = 0,
    NrOfModerators = 1,         // 1 Mod <-> 1 Channel
    NrOfClientsPerChannel = 2,
    TcpListenerPort = 3,
    ClientDspEntry = 4,         // IP of client and port of his DspServer
};

class Config
{
public:
    Config();
    Config(string filename);
    void load();
    void save();
    bool keyExists(string key);
    string getValue(Setting key);
    int getNumber(Setting key);
    void setValue(string key, string value);

private:
    string filename;
    fstream cfg;
    void parseLine(string line);
    map<Setting, string> values;
};



#endif // CONFIG_H
