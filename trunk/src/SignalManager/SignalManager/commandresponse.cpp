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

void commandResponse::setState(bool success)
{
    set("state", success ? "s" : "f");
}

void commandResponse::setInvalidCommand()
{
    setState(false);
    setMessage("invalid command");
}

string commandResponse::getJson()
{
    string result = "{";
    for (std::map<string, string>::iterator it = values.begin(); it != values.end(); ++it)
    {
        result += "\"" + it->first + "\":\"" + it->second + "\"" + (it != values.end() ? "," : "");
    }
    result = result.substr(0, result.length() - 1);
    result += "}";
    return result;
}
