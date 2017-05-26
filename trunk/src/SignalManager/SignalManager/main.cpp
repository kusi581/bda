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
#include "lifecyclemanager.h"
#include "commandhandler.h"

using namespace std;

static Common co;

int main(int argc, char *argv[])
{
    string input;
    co.initLog("MAI", true);
    co.log("Start");

    try {
        dspManager* dspMan = new dspManager();
        dspMan->setupSocket();
        dspMan->startListener();


        multiplexer* multi = multiplexer::Instance();
        multi->init();
        //multi->start(0);

        // testing
        lifecycleManager* cycler = lifecycleManager::Instance();

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
        // std::terminate();
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Caught unknown exception." << std::endl;
    }

    co.log("End");
    return 0;
}
