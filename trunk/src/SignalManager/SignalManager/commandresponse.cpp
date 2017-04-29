#include "commandresponse.h"

commandResponse::commandResponse()
{

}

void commandResponse::set(string key, string value)
{
    values[key] = value;
}

void commandResponse::setMessage(string message)
{
    set("message", message);
}

void commandResponse::addArrayEntry(int index, string key, string value)
{
    isArray = true;
    array[index][key] = value;
}

void commandResponse::setState(bool success)
{
    set("state", success ? "s" : "f");
}

void commandResponse::initArray(int size)
{
    array = vector<map<string,string>>(size);
}

void commandResponse::setInvalidCommand()
{
    setState(false);
    setMessage("invalid command");
}

string commandResponse::getJson()
{
    string result = "";

    if (isArray)
    {
        result += "[";

        for (vector<map<string,string>>::iterator it = array.begin(); it != array.end(); it++)
        {
            result += "{";
            map<string, string> bla = *it;
            for (std::map<string, string>::iterator iit = bla.begin(); iit != bla.end(); iit++)
            {
                result += "\"" + iit->first + "\":\"" + iit->second + "\",";
            }
            result = result.substr(0, result.length() - 1);
            result += "}";
            result += ",";
        }

        result = result.substr(0, result.length() - 1);
        result += "]";
    }
    else
    {
        result += "{";
        for (std::map<string, string>::iterator it = values.begin(); it != values.end(); ++it)
        {
            result += "\"" + it->first + "\":\"" + it->second + "\"" + (it != values.end() ? "," : "");
        }
        result = result.substr(0, result.length() - 1);
        result += "}";
    }

    return result;
}
