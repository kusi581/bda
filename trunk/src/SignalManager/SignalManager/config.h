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
    void load();
    void save();
    bool keyExists(string key);
    string getValue(string key);
    int getNumber(string key);
    void setValue(string key, string value);
    string getValue(string key, int part);
    int getNumber(string key, int part);

private:
    Common co;
    string filename;
    fstream cfg;
    void parseLine(string line);
    map<string, string> values;
};



#endif // CONFIG_H
