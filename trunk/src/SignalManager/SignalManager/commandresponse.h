#ifndef COMMANDRESPONSE_H
#define COMMANDRESPONSE_H

#include <map>
#include <string>

using namespace std;

class commandResponse
{
public:
    commandResponse();

    void set(string key, string value);
    void setMessage(string message);

    void setInvalidCommand();
    void setState(bool success);

    string getJson();

private:
    map<string, string> values;
};

#endif // COMMANDRESPONSE_H
