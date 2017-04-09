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

using namespace std;

Config cfg = Config("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/SignalManager.cfg");
int isRunning = true;
char* tLog;

int main(int argc, char *argv[])
{
    string input;
    Common::log("Start");

    //cfg.loadFromFile("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/SiMa.cfg");
    //cfg.saveTo("/home/kusi/School/bda/repo/trunk/src/SignalManager/SignalManager/SiMa.cfg");

    std::thread tcpThread(&tcpListener);
    std::thread udpThread(&udpListener);

    while(true){
        cin >> input;
        if (input == "q")
        {
            isRunning = false;
            break;
        }
    }

    // stop them threads
    //tcpThread.join();
    std::terminate();

    Common::log("End");
    return 0;
}

// 1 Listener/DSP-Master (used for multiplexing IQ Data)
void udpListener(){
    int sock;
    struct sockaddr_in server, client;
    unsigned char udpReceiveBuffer[512];
    int bytes_read;
    unsigned int length;

    Common::log("tcpListener started");

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1){
        Common::log("Error while creating udp socket");
        return;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(13000 + 0); // add offset depending on dsp master

    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == -1)
    {
        Common::log("Error while binding socket to port (udp)");
        return;
    }
    Common::log("udp Socket created");

    while (isRunning) {
        bytes_read = recvfrom(sock, udpReceiveBuffer, sizeof(udpReceiveBuffer), 0, (struct sockaddr*)&client, &length);
        if (bytes_read < 0){
            Common::log("Error while reading from udp socket.");

            break;
        }
        sprintf(tLog,"[LOG] udp Message received from %s:d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        Common::log(tLog);
    }

    Common::log("tcpListener stopped");
}

// receiving commands from webserver
void tcpListener(){
    int sock;
    struct sockaddr_in server;
    unsigned char tcpReceiveBuffer[512];
    int bytes_read;
    unsigned int length;

    // create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        Common::log("Error while creating tcp socket!");
        return;
    }
    Common::log("tcp Socket created");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(cfg.getNumber(TcpListenerPort));

    while (isRunning){
        bytes_read = recvfrom(sock, tcpReceiveBuffer, sizeof(tcpReceiveBuffer), 0, (struct sockaddr *)&server,&length);

        if (bytes_read < 0)
        {
            //Common::log("Error while reading from tcp socket.");
            //break;
        }
        else
        {
            Common::log("tcp Message received");
        }
    }
    Common::log("tcpListener stopped");
}
