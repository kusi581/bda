#ifndef COMMANDRESPONSE_H
#define COMMANDRESPONSE_H

#include <map>
#include <string>
#include <vector>

using namespace std;

class commandResponse
{
public:
    commandResponse();

    void set(string key, string value);
    void setMessage(string message);

    void setInvalidCommand();
    void setState(bool success);

    void initArray(int size);
    void addArrayEntry(int index, string key, string value);

    string getJson();

private:
    map<string, string> values;
    vector<map<string, string>> array;
    bool isArray;
};

#endif // COMMANDRESPONSE_H
