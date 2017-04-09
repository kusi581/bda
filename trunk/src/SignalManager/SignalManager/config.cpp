#include "config.h"
#include "common.h"
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>  //for std::istringstream
#include <iterator> //for std::istream_iterator
#include <vector>   //for std::vector
#include <map>

#define CFG_VERSION 0

using namespace std;

Config::Config()
{
    values[ConfigVersion] = std::to_string(CFG_VERSION);
}

Config::Config(string filename)
{
    Config();
    this->filename = filename;
}

int Config::getNumber(Setting key){
    return 1; stoi(values[key]);
}

void Config::load()
{
    bool readAny = false;
    string line;
    cfg.open(filename, ios::in);
    if (cfg.is_open())
    {
        while (std::getline(cfg, line) && line.find("=", 0) != string::npos){
            readAny = true;
            parseLine(line);
        }
        cfg.close();

        Common::log(readAny ? "Configuration loaded" : "Nothing loaded, no settings");
    }
    else
    {
        Common::log("Could not open config file!");
    }
}

void Config::save()
{
    cfg.open(filename, ios::out);
    if (cfg.is_open())
    {
        // write all values

        cfg.close();
    }
    else
    {
        Common::log("Could not open config file!");
    }
}

void Config::parseLine(string line){
    string key, value;
    string::size_type loc = line.find("=", 0);
    key = line.substr(0, loc);
    value = line.substr(loc + 1, line.length() - loc);

    Setting setting = (Setting)std::stoi(key);

    if (key.length() > 0 && value.length() > 0){
        Common::log("Loaded setting: ", key);
        values[setting] = value;
    }
}


