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
        
        while (running.load()) 
        {
            sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket >= 0) 
            {
                handleClient(clientSocket);
                close(clientSocket);
            }
        }
        
        close(serverSocket);
    });
}

void WebServer::stop() 
{
    running.store(false);
    if (serverThread.joinable()) 
    {
        serverThread.join();
    }
}

void WebServer::handleClient(int clientSocket) 
{
    char buffer[4096] = {0};
    read(clientSocket, buffer, sizeof(buffer));
    
    string request(buffer);
    string response;
    
    if (request.find("GET / ") != string::npos) 
    {
        ifstream file("../web_gui/index.html");
        if (file.is_open()) 
        {
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
        bool processesExist = stats.totalProcesses > 0;  // True if any processes are defined in the system
        string json = "{\"time\":" + to_string(stats.currentTime) +
                     ",\"hasProcesses\":" + (scheduler->hasProcesses() ? "true" : "false") +
                     ",\"isComplete\":" + (scheduler->isComplete() ? "true" : "false") +
                     ",\"processesExist\":" + (processesExist ? "true" : "false") +
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
    } 
    else if (request.find("GET /api/processes") != string::npos) 
    {
        string json = "{";
        auto currentProcess = scheduler->getCurrentProcess();
        if (currentProcess && currentProcess->getState() == ProcessState::RUNNING) 
        {
            json += "\"currentRunningPid\":" + to_string(currentProcess->getPid()) + ",";
        } 
        else 
        {
            json += "\"currentRunningPid\":null,";
        }
        json += "\"processes\":[";
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
                   ",\"completion\":" + to_string(p->getCompletionTime()) +
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
        switch(setNumber) 
        {
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
            case 5: // Mixed Workload: Mix of short and long processes
                scheduler->addProcess(0, 5);
                scheduler->addProcess(2, 30);
                scheduler->addProcess(4, 10);
                scheduler->addProcess(6, 45);
                scheduler->addProcess(8, 2);
                break;
            case 6: // Simultaneous Arrival: All processes arrive at the same time
                scheduler->addProcess(0, 10);
                scheduler->addProcess(0, 5);
                scheduler->addProcess(0, 20);
                scheduler->addProcess(0, 3);
                scheduler->addProcess(0, 15);
                break;
            case 7: // Gradual Buildup: Increasing number of processes over time
                scheduler->addProcess(0, 10);
                scheduler->addProcess(1, 9);
                scheduler->addProcess(3, 8);
                scheduler->addProcess(6, 7);
                scheduler->addProcess(10, 6);
                scheduler->addProcess(15, 5);
                break;
            case 8: // Priority Test: Processes with clear priority distinctions (MLFQS usually handles this via feedback)
                // Assuming MLFQS handles priority implicitly; here we use arrival/burst times to influence the scheduler's behavior
                scheduler->addProcess(0, 5);  
                scheduler->addProcess(0, 50); 
                scheduler->addProcess(1, 10);
                scheduler->addProcess(2, 40);
                scheduler->addProcess(3, 3);
                break;
            default: // Default to Standard Set
                scheduler->addProcess(0, 20);
                scheduler->addProcess(5, 12);
                scheduler->addProcess(10, 8);
                scheduler->addProcess(15, 16);
                scheduler->addProcess(20, 5);
        }

        // Check for new arrivals to properly update process queues
        scheduler->checkNewArrivals();

        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 15\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    } 
    else if (request.find("POST /api/random") != string::npos) 
    {
        // Parse POST data for random process parameters
        size_t bodyStart = request.find("\r\n\r\n");

        // Set default values
        int count = 5;         // default number of processes
        int minArrival = 0;    // default min arrival time
        int maxArrival = 10;   // default max arrival time
        int minBurst = 1;      // default min burst time
        int maxBurst = 15;     // default max burst time

        if (bodyStart != string::npos) 
        {
            string body = request.substr(bodyStart + 4);

            // Parse all parameters from the body

            // Split the body by '&' to get individual parameters
            stringstream ss(body);
            string item;
            while(getline(ss, item, '&')) 
            {
                size_t eqPos = item.find('=');
                if (eqPos != string::npos) 
                {
                    string key = item.substr(0, eqPos);
                    string value = item.substr(eqPos + 1);

                    if (key == "count") 
                    {
                        try 
                        {
                            count = stoi(value);
                            // Clamp count to a reasonable range
                            if (count < 1) count = 1;
                            if (count > 50) count = 50;
                        } 
                        catch (...) 
                        {
                            // Keep default value if conversion fails
                        }
                    } 
                    else if (key == "minArrival")
                    {
                        try
                        {
                            minArrival = stoi(value);
                            // Clamp min arrival to a reasonable range
                            if (minArrival < 0) minArrival = 0;
                            if (minArrival > 100) minArrival = 100;
                        }
                        catch (...)
                        {
                            // Keep default value if conversion fails
                        }
                    }
                    else if (key == "maxArrival")
                    {
                        try
                        {
                            maxArrival = stoi(value);
                            // Clamp max arrival to a reasonable range
                            if (maxArrival < 0) maxArrival = 0;
                            if (maxArrival > 100) maxArrival = 100;
                        }
                        catch (...)
                        {
                            // Keep default value if conversion fails
                        }
                    }
                    else if (key == "minBurst") 
                    {
                        try 
                        {
                            minBurst = stoi(value);
                            // Clamp min burst to a reasonable range
                            if (minBurst < 1) minBurst = 1;
                            if (minBurst > 50) minBurst = 50;
                        } 
                        catch (...) 
                        {
                            // Keep default value if conversion fails
                        }
                    } 
                    else if (key == "maxBurst") 
                    {
                        try 
                        {
                            maxBurst = stoi(value);
                            // Clamp max burst to a reasonable range
                            if (maxBurst < 1) maxBurst = 1;
                            if (maxBurst > 100) maxBurst = 100;
                        } catch (...) {
                            // Keep default value if conversion fails
                        }
                    }
                }
            }
        }

        // Validate minArrival <= maxArrival
        if (minArrival > maxArrival)
        {
            int temp = minArrival;
            minArrival = maxArrival;
            maxArrival = temp;
        }

        // Validate minBurst <= maxBurst
        if (minBurst > maxBurst)
        {
            int temp = minBurst;
            minBurst = maxBurst;
            maxBurst = temp;
        }

        // Add random processes with the user-specified parameters
        srand(time(nullptr));
        for (int i = 0; i < count; i++)
        {
            int arrival = (rand() % (maxArrival - minArrival + 1)) + minArrival;  // minArrival to maxArrival
            int burst = (rand() % (maxBurst - minBurst + 1)) + minBurst;  // minBurst to maxBurst
            scheduler->addProcess(arrival, burst);
        }

        // Check for new arrivals to properly update process queues
        scheduler->checkNewArrivals();

        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 15\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    } 
    else if (request.find("POST /api/step") != string::npos) 
    {
        scheduler->step();
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 15\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    }
     else if (request.find("POST /api/reset") != string::npos) 
     {
        scheduler->reset();
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 15\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    } 
    else if (request.find("POST /api/add-process") != string::npos) 
    {
        // Parse POST data for arrival and burst time
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart != string::npos) 
        {
            string body = request.substr(bodyStart + 4);

            // Simple parsing - expect "arrival=X&burst=Y"
            size_t arrivalPos = body.find("arrival=");
            size_t burstPos = body.find("burst=");
            if (arrivalPos != string::npos && burstPos != string::npos) 
            {
                int arrival = stoi(body.substr(arrivalPos + 8, body.find("&", arrivalPos) - arrivalPos - 8));
                int burst = stoi(body.substr(burstPos + 6));
                scheduler->addProcess(arrival, burst);
            }
        }

        // Check for new arrivals to properly update process queues
        scheduler->checkNewArrivals();

        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Content-Length: 15\r\n";
        response += "\r\n";
        response += "{\"success\":true}";
    }
     else if (request.find("POST /api/config") != string::npos) 
     {
        // Parse POST data for configuration
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart != string::npos) 
        {
            string body = request.substr(bodyStart + 4);

            // Create default config based on current settings
            SchedulerConfig newConfig = scheduler->getConfig();

            // Parse all configuration parameters
            bool boostEnabled = true; // Default is enabled

            // Split the body by '&' to get individual parameters
            stringstream ss(body);
            string item;
            while(getline(ss, item, '&')) 
            {
                size_t eqPos = item.find('=');
                if (eqPos != string::npos) 
                {
                    string key = item.substr(0, eqPos);
                    string value = item.substr(eqPos + 1);

                    if (key == "algorithm") 
                    {
                        if (value == "sjf") 
                        {
                            scheduler->setLastQueueAlgorithm(LastQueueAlgorithm::SHORTEST_JOB_FIRST);
                            cout << "New last queue algorithm: Shortest Job First" << endl;
                        } 
                        else if (value == "priority") 
                        {
                            scheduler->setLastQueueAlgorithm(LastQueueAlgorithm::PRIORITY_SCHEDULING);
                            cout << "New last queue algorithm: Priority Scheduling" << endl;
                        }
                         else 
                         {
                            scheduler->setLastQueueAlgorithm(LastQueueAlgorithm::ROUND_ROBIN);
                            cout << "New last queue algorithm: Round Robin" << endl;
                        }
                    } 
                    else if (key == "boost") 
                    {
                        // Handle the boost checkbox - if false, disable boost
                        if (value == "false") 
                        {
                            boostEnabled = false;
                        }
                    } 
                    else if (key == "interval") 
                    {
                        try 
                        {
                            newConfig.boostInterval = stoi(value);
                        }
                         catch (...) 
                         {
                            // Keep default value if conversion fails
                        }
                    }
                     else if (key == "numQueues") 
                     {
                        try 
                        {
                            newConfig.numQueues = stoi(value);
                            // Ensure the value is within valid range
                            if (newConfig.numQueues < 2) newConfig.numQueues = 2;
                            if (newConfig.numQueues > 5) newConfig.numQueues = 5;
                        }
                         catch (...) 
                         {
                            // Keep default value if conversion fails
                        }
                    } 
                    else if (key == "baseQuantum") 
                    {
                        try 
                        {
                            newConfig.baseQuantum = stoi(value);
                            // Ensure the value is within valid range
                            if (newConfig.baseQuantum < 1) newConfig.baseQuantum = 1;
                            if (newConfig.baseQuantum > 10) newConfig.baseQuantum = 10;
                        } 
                        catch (...) 
                        {
                            // Keep default value if conversion fails
                        }
                    } 
                    else if (key == "quantumMultiplier")
                    {
                        try
                        {
                            newConfig.quantumMultiplier = stod(value);
                            // Ensure the value is within valid range
                            if (newConfig.quantumMultiplier < 1.0) newConfig.quantumMultiplier = 1.0;
                            if (newConfig.quantumMultiplier > 5.0) newConfig.quantumMultiplier = 5.0;
                        }
                         catch (...)
                         {
                            // Keep default value if conversion fails
                        }
                    }
                    else if (key == "speed")
                    {
                        // Convert speed value (1-10) to animation delay (lower = faster)
                        // Speed 1 = slowest (1000ms delay), Speed 10 = fastest (100ms delay)
                        try
                        {
                            int speedValue = stoi(value);
                            // Map speed (1-10) to animation delay (1000ms to 100ms)
                            newConfig.animationDelay = 1100 - (speedValue * 100); // 1000 to 100
                            // Ensure it's within valid range
                            if (newConfig.animationDelay < 50) newConfig.animationDelay = 50;
                            if (newConfig.animationDelay > 2000) newConfig.animationDelay = 2000;
                        }
                        catch (...)
                        {
                            // Keep default value if conversion fails
                        }
                    }
                }
            }

            // Apply the boost setting: if disabled, set interval to -1; otherwise use the interval
            if (!boostEnabled) 
            {
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
    } 
    else 
    {
        string content = "404 Not Found";
        response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: " + to_string(content.length()) + "\r\n";
        response += "\r\n";
        response += content;
    }
    
    send(clientSocket, response.c_str(), response.length(), 0);
}
