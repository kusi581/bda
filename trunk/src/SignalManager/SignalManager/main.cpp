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

char* tLog;
static Common co;

int main(int argc, char *argv[])
{
    string input;
    co.initLog("mai", true);
    co.log("Start");

    //std::thread tcpThread(&tcpListener);
    //std::thread udpThread(&udpListener);

    dspManager* dspMan = new dspManager();
    dspMan->setupSocket();
    dspMan->startListener();

    multiplexer* multi = new multiplexer();
    multi->init(2000);
    multi->start();

    while(true){
        cin >> input;
        if (input == "q")
        {
            dspMan->stopListener();
            break;
        }
    }

    // stop them threads
    //tcpThread.join();
    std::terminate();

    co.log("End");
    return 0;
}

// 1 Listener/DSP-Master (used for multiplexing IQ Data)
void udpListener(){
    int sock;
    struct sockaddr_in server, client;
    unsigned char udpReceiveBuffer[512];
    int bytes_read;
    unsigned int length;

    co.log("udpListener started");

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1){
        co.log("Error while creating udp socket");
        return;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(8000); // add offset depending on dsp master

    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == -1)
    {
        co.log("Error while binding socket to port (udp)");
        return;
    }
    co.log("udp Socket created");

    while (true) {
        bytes_read = recvfrom(sock, udpReceiveBuffer, sizeof(udpReceiveBuffer), 0, (struct sockaddr*)&client, &length);
        if (bytes_read < 0){
            co.log("Error while reading from udp socket.");

            break;
        }
        sprintf(tLog,"[LOG] udp Message received from %s:d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        co.log(tLog);
    }

    co.log("udpListener stopped");
}

// receiving commands from webserver
void tcpListener(){

}
