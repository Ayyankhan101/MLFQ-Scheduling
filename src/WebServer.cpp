#include "WebServer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
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
