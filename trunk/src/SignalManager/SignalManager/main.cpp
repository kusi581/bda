/*#include <ctime>
#include <iostream>
#include <iomanip>
#include <stdio.h> //printf
#include <stdlib.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <pthread.h>*/

#include "common.h"
#include "config.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include "main.h"
#include "dspmanager.h"
#include "multiplexer.h"

using namespace std;

static Common co;

int main(int argc, char *argv[])
{
    string input;
    co.initLog("MAI", true);
    co.log("Start");

    dspManager* dspMan = new dspManager();
    dspMan->setupSocket();
    dspMan->startListener();

    multiplexer* multi = new multiplexer();
    multi->init();
    multi->start(0);
    //multi->start(1);

    while(true){
        cin >> input;
        if (input == "q")
        {
            multi->stop();
            dspMan->stopListener();
            break;
        }
    }

    // stop them threads
    std::terminate();

    co.log("End");
    return 0;
}
