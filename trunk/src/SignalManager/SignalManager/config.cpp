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

using namespace std;

Config::Config()
{
    // setting default values
    tcpListenPort = 44444;
    nrOfModerators = 2;
    nrOfSlavesPerModerator = 4;
}

void Config::loadFrom(string filename)
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

        Common::log(readAny ? "Configuration loaded with the following values: " : "Nothing loaded, using default values:");
        Common::log(std::to_string(tcpListenPort));
        Common::log(std::to_string(nrOfModerators));
        Common::log(std::to_string(nrOfSlavesPerModerator));
    }
    else
    {
        Common::log("Could not open config file!");
    }
}

void Config::saveTo(string filename)
{
    cfg.open(filename, ios::out);
    if (cfg.is_open())
    {
        cfg << "tcpListenPort=" << tcpListenPort << endl;
        cfg << "nrOfModerators=" << nrOfModerators << endl;
        cfg << "nrOfSlavesPerModerator=" << nrOfSlavesPerModerator << endl;
        cfg.close();
    }
    else
    {
        Common::log("Could not open config file!");
    }
}

void Config::parseLine(string line){
    int valueInt;
    string key, value;
    string::size_type loc = line.find("=", 0);
    key = line.substr(0, loc);
    value = line.substr(loc + 1, line.length() - loc);

    if (sscanf(value.c_str(), "%d", &valueInt) == EOF)
        Common::log("Value is not a number");

    if (key == "tcpListenPort")
        tcpListenPort = valueInt;
    else if (key == "nrOfModerators")
        nrOfModerators = valueInt;
    else if (key == "nrOfSlavesPerModerator")
        nrOfSlavesPerModerator = valueInt;
}


