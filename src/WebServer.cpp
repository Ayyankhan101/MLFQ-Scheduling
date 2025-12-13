#include "WebServer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
using namespace std;

WebServer::WebServer(MLFQScheduler* sched, int port) 
    : scheduler(sched), running(false), port(port) {}

WebServer::~WebServer() 
{
    stop();
}

void WebServer::start() 
{
    if (running.load()) return;
    
    running.store(true);
    serverThread = thread([this]() 
    {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            cerr << "Error creating socket\n";
            return;
        }
        
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        
        // Try multiple ports if the default is busy
        int currentPort = port;
        bool bound = false;
        for (int i = 0; i < 10; i++) 
        {
            serverAddr.sin_port = htons(currentPort);
            if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) >= 0) {
                port = currentPort;
                bound = true;
                break;
            }
            currentPort++;
        }
        
        if (!bound) {
            cerr << "Error: Could not bind to any port from " << port << " to " << (port + 9) << "\n";
            close(serverSocket);
            return;
        }
        
        if (listen(serverSocket, 5) < 0) {
            cerr << "Error listening on socket\n";
            close(serverSocket);
            return;
        }
        
        cout << "Web server started on http://localhost:" << port << "\n";
        
        while (running.load()) {
            sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket >= 0) {
                handleClient(clientSocket);
                close(clientSocket);
            }
        }
        
        close(serverSocket);
    });
}

void WebServer::stop() {
    running.store(false);
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

void WebServer::handleClient(int clientSocket) {
    char buffer[4096] = {0};
    read(clientSocket, buffer, sizeof(buffer));
    
    string request(buffer);
    string response;
    
    if (request.find("GET / ") != string::npos) {
        ifstream file("../web_gui/index.html");
        if (file.is_open()) {
            stringstream ss;
            ss << file.rdbuf();
            string content = ss.str();
            
            response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/html\r\n";
            response += "Content-Length: " + to_string(content.length()) + "\r\n";
            response += "\r\n";
            response += content;
        } else {
            string content = "Web GUI files not found. Make sure web_gui/ directory exists.";
            response = "HTTP/1.1 404 Not Found\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Content-Length: " + to_string(content.length()) + "\r\n";
            response += "\r\n";
            response += content;
        }
    } else if (request.find("GET /styles.css") != string::npos) {
        ifstream file("../web_gui/styles.css");
        if (file.is_open()) {
            stringstream ss;
            ss << file.rdbuf();
            string content = ss.str();
            
            response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/css\r\n";
            response += "Content-Length: " + to_string(content.length()) + "\r\n";
            response += "\r\n";
            response += content;
        }
    } else if (request.find("GET /script.js") != string::npos) {
        ifstream file("../web_gui/script.js");
        if (file.is_open()) {
            stringstream ss;
            ss << file.rdbuf();
            string content = ss.str();
            
            response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: application/javascript\r\n";
            response += "Content-Length: " + to_string(content.length()) + "\r\n";
            response += "\r\n";
            response += content;
        }
    } else if (request.find("GET /api/status") != string::npos) {
        auto stats = scheduler->getStats();
        int lastBoostTime = scheduler->getCurrentTime() - scheduler->getBoostTimer();
        string json = "{\"time\":" + to_string(stats.currentTime) + 
                     ",\"hasProcesses\":" + (scheduler->hasProcesses() ? "true" : "false") +
                     ",\"completedProcesses\":" + to_string(stats.completedProcesses) +
                     ",\"totalProcesses\":" + to_string(stats.totalProcesses) +
                     ",\"cpuUtilization\":" + to_string(stats.cpuUtilization) +
                     ",\"avgWaitTime\":" + to_string(stats.avgWaitTime) +
                     ",\"avgTurnaroundTime\":" + to_string(stats.avgTurnaroundTime) +
                     ",\"avgResponseTime\":" + to_string(stats.avgResponseTime) +
                     ",\"boostInterval\":" + to_string(scheduler->getBoostInterval()) +
                     ",\"boostEnabled\":" + (scheduler->getBoostInterval() > 0 ? "true" : "false") +
                     ",\"lastBoostTime\":" + to_string(lastBoostTime) +
                     ",\"nextBoostIn\":" + to_string(scheduler->getNextBoostIn()) + "}";
        
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: " + to_string(json.length()) + "\r\n";
        response += "\r\n";
        response += json;
    } else if (request.find("GET /api/processes") != string::npos) {
        string json = "{\"processes\":[";
        auto processes = scheduler->getAllProcesses();
        for (size_t i = 0; i < processes.size(); i++) {
            auto p = processes[i];
            string status = "Ready";
            if (p->getState() == ProcessState::TERMINATED) status = "Completed";
            else if (p->getState() == ProcessState::RUNNING) status = "Running";
            
            json += "{\"pid\":" + to_string(p->getPid()) +
                   ",\"arrival\":" + to_string(p->getArrivalTime()) +
                   ",\"burst\":" + to_string(p->getBurstTime()) +
                   ",\"remaining\":" + to_string(p->getRemainingTime()) +
                   ",\"queue\":" + to_string(p->getPriority()) +
                   ",\"status\":\"" + status + "\"}";
            if (i < processes.size() - 1) json += ",";
        }
        json += "]}";
        
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: " + to_string(json.length()) + "\r\n";
        response += "\r\n";
        response += json;
    } else if (request.find("GET /api/queues") != string::npos) {
        string json = "{\"queues\":[";
        auto queues = scheduler->getQueues();
        for (size_t i = 0; i < queues.size(); i++) {
            json += "{\"id\":" + to_string(i) +
                   ",\"size\":" + to_string(queues[i].size()) +
                   ",\"quantum\":" + to_string(queues[i].getTimeQuantum()) +
                   ",\"processes\":[";

            // Add process IDs in this queue
            auto queueProcesses = queues[i].getProcesses();
            for (size_t j = 0; j < queueProcesses.size(); j++) {
                json += to_string(queueProcesses[j]->getPid());
                if (j < queueProcesses.size() - 1) json += ",";
            }
            json += "]}";

            if (i < queues.size() - 1) json += ",";
        }
        json += "]}";

        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: " + to_string(json.length()) + "\r\n";
        response += "\r\n";
        response += json;
    } else if (request.find("POST /api/preset") != string::npos) {
        // Parse preset set number from request body
        int setNumber = 1; // default
        size_t setPos = request.find("set=");
        if (setPos != string::npos) {
            setNumber = stoi(request.substr(setPos + 4, 1));
        }
        
        // Load different preset sets based on setNumber
        switch(setNumber) {
            case 1: // Standard Set
                scheduler->addProcess(0, 20);
                scheduler->addProcess(5, 12);
                scheduler->addProcess(10, 8);
                scheduler->addProcess(15, 16);
                scheduler->addProcess(20, 5);
                break;
            case 2: // CPU-Intensive
                scheduler->addProcess(0, 30);
                scheduler->addProcess(5, 25);
                scheduler->addProcess(10, 20);
                break;
            case 3: // I/O-Intensive
                scheduler->addProcess(0, 3);
                scheduler->addProcess(2, 2);
                scheduler->addProcess(4, 4);
                scheduler->addProcess(6, 3);
                scheduler->addProcess(8, 2);
                break;
            case 4: // Mixed Workload
                scheduler->addProcess(0, 15);
                scheduler->addProcess(2, 3);
                scheduler->addProcess(4, 8);
                scheduler->addProcess(6, 2);
                scheduler->addProcess(8, 12);
                scheduler->addProcess(10, 5);
                scheduler->addProcess(12, 7);
                break;
            default: // Default to Standard Set
                scheduler->addProcess(0, 20);
                scheduler->addProcess(5, 12);
                scheduler->addProcess(10, 8);
                scheduler->addProcess(15, 16);
                scheduler->addProcess(20, 5);
        }
        
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 15\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    } else if (request.find("POST /api/random") != string::npos) {
        // Add random processes
        srand(time(nullptr));
        for (int i = 0; i < 5; i++) {
            scheduler->addProcess(rand() % 10, (rand() % 15) + 1);
        }
        
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 15\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    } else if (request.find("POST /api/step") != string::npos) {
        scheduler->step();
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 15\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    } else if (request.find("POST /api/reset") != string::npos) {
        scheduler->reset();
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 15\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    } else if (request.find("POST /api/add-process") != string::npos) {
        // Parse POST data for arrival and burst time
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart != string::npos) {
            string body = request.substr(bodyStart + 4);
            // Simple parsing - expect "arrival=X&burst=Y"
            size_t arrivalPos = body.find("arrival=");
            size_t burstPos = body.find("burst=");
            if (arrivalPos != string::npos && burstPos != string::npos) {
                int arrival = stoi(body.substr(arrivalPos + 8, body.find("&", arrivalPos) - arrivalPos - 8));
                int burst = stoi(body.substr(burstPos + 6));
                scheduler->addProcess(arrival, burst);
            }
        }
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 15\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    } else if (request.find("POST /api/config") != string::npos) {
        // Parse POST data for configuration
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart != string::npos) {
            string body = request.substr(bodyStart + 4);

            // Create default config based on current settings
            SchedulerConfig newConfig = scheduler->getConfig();

            // Parse all configuration parameters
            size_t pos = 0;
            string param;
            bool boostEnabled = true; // Default is enabled

            // Split the body by '&' to get individual parameters
            stringstream ss(body);
            string item;
            while(getline(ss, item, '&')) {
                size_t eqPos = item.find('=');
                if (eqPos != string::npos) {
                    string key = item.substr(0, eqPos);
                    string value = item.substr(eqPos + 1);

                    if (key == "algorithm") {
                        if (value == "sjf") {
                            scheduler->setLastQueueAlgorithm(LastQueueAlgorithm::SHORTEST_JOB_FIRST);
                            cout << "New last queue algorithm: Shortest Job First" << endl;
                        } else if (value == "priority") {
                            scheduler->setLastQueueAlgorithm(LastQueueAlgorithm::PRIORITY_SCHEDULING);
                            cout << "New last queue algorithm: Priority Scheduling" << endl;
                        } else {
                            scheduler->setLastQueueAlgorithm(LastQueueAlgorithm::ROUND_ROBIN);
                            cout << "New last queue algorithm: Round Robin" << endl;
                        }
                    } else if (key == "boost") {
                        // Handle the boost checkbox - if false, disable boost
                        if (value == "false") {
                            boostEnabled = false;
                        }
                    } else if (key == "interval") {
                        try {
                            newConfig.boostInterval = stoi(value);
                        } catch (...) {
                            // Keep default value if conversion fails
                        }
                    } else if (key == "numQueues") {
                        try {
                            newConfig.numQueues = stoi(value);
                            // Ensure the value is within valid range
                            if (newConfig.numQueues < 2) newConfig.numQueues = 2;
                            if (newConfig.numQueues > 5) newConfig.numQueues = 5;
                        } catch (...) {
                            // Keep default value if conversion fails
                        }
                    } else if (key == "baseQuantum") {
                        try {
                            newConfig.baseQuantum = stoi(value);
                            // Ensure the value is within valid range
                            if (newConfig.baseQuantum < 1) newConfig.baseQuantum = 1;
                            if (newConfig.baseQuantum > 10) newConfig.baseQuantum = 10;
                        } catch (...) {
                            // Keep default value if conversion fails
                        }
                    } else if (key == "quantumMultiplier") {
                        try {
                            newConfig.quantumMultiplier = stod(value);
                            // Ensure the value is within valid range
                            if (newConfig.quantumMultiplier < 1.0) newConfig.quantumMultiplier = 1.0;
                            if (newConfig.quantumMultiplier > 5.0) newConfig.quantumMultiplier = 5.0;
                        } catch (...) {
                            // Keep default value if conversion fails
                        }
                    }
                }
            }

            // Apply the boost setting: if disabled, set interval to -1; otherwise use the interval
            if (!boostEnabled) {
                newConfig.boostInterval = -1; // Disable boost
            }
            // If boostEnabled is true, interval keeps its value from the "interval" parameter

            // Update the scheduler with the new configuration
            scheduler->updateConfig(newConfig);
        }

        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 17\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    } else {
        string content = "404 Not Found";
        response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: " + to_string(content.length()) + "\r\n";
        response += "\r\n";
        response += content;
    }
    
    send(clientSocket, response.c_str(), response.length(), 0);
}
