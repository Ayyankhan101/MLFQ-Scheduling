#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "MLFQScheduler.h"
#include <string>
#include <thread>
#include <atomic>
using namespace std;


class WebServer {
private:
    MLFQScheduler* scheduler;
    atomic<bool> running;
    thread serverThread;
    int port;
    
    string generateHTML();
    string processRequest(const string& request);
    void handleClient(int clientSocket);
    
public:
    WebServer(MLFQScheduler* sched, int port = 8080);
    ~WebServer();
    
    void start();
    void stop();
    bool isRunning() const { return running.load(); }
    int getPort() const { return port; }
};

#endif
