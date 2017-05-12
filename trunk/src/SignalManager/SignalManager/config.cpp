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
    co.initLog("CFG", false);
    values["ConfigVersion"] = std::to_string(CFG_VERSION);
    saveOnChange = true;
}

Config::Config(string filename)
{
    Config();
    this->filename = filename;
}

bool Config::exists()
{
    bool exists = false;
    cfg.open(filename, ios::in);

    if (cfg.is_open())
    {
        exists = true;
        cfg.close();
    }
    return exists;
}

int Config::getNumber(string key){ 
    string val = getValue(key);
    if (val.length() > 0)
    {
        return stoi(val);
    }
    else
    {
        co.log("Value is empty: " + key);
        return 0;
    }
}

void Config::setValue(string key, string value)
{
    values[key] = value;

    // save when something changes
    if (saveOnChange)
        save();
}

void Config::setValue(string key, string partValue, int part)
{
    string value = getValue(key);
    std::vector<std::string> parts = co.split(value, ',');
    parts[part] = partValue;

    value = co.join(parts, ",");
    setValue(key, value);
}

int Config::getNumber(string key, int part){
    string val = getValue(key, part);
    if (val.length() > 0)
    {
        return stoi(val);
    }
    else
    {
        co.log("Value is empty: " + key);
        return 0;
    }
}

bool Config::keyExists(string key)
{
    std::map<string, string>::iterator pos = values.find(key);
    return pos != values.end();
}

string Config::getValue(string key)
{
    std::map<string, string>::iterator pos = values.find(key);
    return keyExists(key) ? pos->second : "";
}

string Config::getValue(string key, int part)
{
    string val = getValue(key);
    std::vector<std::string> parts = co.split(val, ',');
    return parts.size() == 0 ? "" : parts[part];
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

        co.log(readAny ? "Configuration loaded" : "Nothing loaded, no settings");
    }
    else
    {
        co.log("Could not open config file!");
    }
}

void Config::save()
{
    cfg.open(filename, ios::out);
    if (cfg.is_open())
    {
        // write all values
        for (std::map<string, string>::iterator it = values.begin(); it != values.end(); ++it)
        {
            cfg << it->first << "=" << it->second << endl;
        }

        cfg.close();
    }
    else
    {
        co.log("Could not open config file!");
    }
}

void Config::enableSaveOnChange(bool saveEnabled)
{
    saveOnChange = saveEnabled;
}

void Config::parseLine(string line){
    string key, value;
    string::size_type loc = line.find("=", 0);
    key = line.substr(0, loc);
    value = line.substr(loc + 1, line.length() - loc);

    if (key.length() > 0 && value.length() > 0){
        co.log(key, value);
        values[key] = value;
    }
}


