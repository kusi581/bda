#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <map>
using namespace std;

class Config
{
public:
    Config();
    Config(string filename);
    bool exists();
    void load();
    void save();
    void enableSaveOnChange(bool saveEnabled);

    bool keyExists(string key);

    string getValue(string key);
    string getValue(string key, int part);

    int getNumber(string key);
    int getNumber(string key, int part);

    void setValue(string key, string value);
    void setValue(string key, string value, int part);
private:
    Common co;
    string filename;
    fstream cfg;
    void parseLine(string line);
    map<string, string> values;
    bool saveOnChange;
};



#endif // CONFIG_H
