# WebServer and Web GUI Documentation

## Table of Contents
1. [WebServer.h - Header File](#webserversh-header-file)
2. [WebServer.cpp - Implementation](#webservercpp-implementation)
3. [API Endpoints](#api-endpoints)
4. [Web GUI Structure](#web-gui-structure)
5. [Interaction Between Backend and Frontend](#interaction-between-backend-and-frontend)

---

## WebServer.h - Header File

The `WebServer.h` header file defines the interface for the built-in HTTP server that serves the web-based MLFQ scheduler interface.

### Class Definition

```cpp
class WebServer {
private:
    MLFQScheduler* scheduler;  // Pointer to the MLFQ scheduler instance
    atomic<bool> running;      // Flag indicating if the server is running
    thread serverThread;       // Thread for handling HTTP server operations
    int port;                  // Port on which the server listens (default: 8080)

    string generateHTML();                 // Not currently used - likely legacy method
    string processRequest(const string& request); // Not currently used - likely legacy method
    void handleClient(int clientSocket);   // Handles individual client connections

public:
    WebServer(MLFQScheduler* sched, int port = 8080);  // Constructor
    ~WebServer();                                        // Destructor
    void start();                                        // Start the web server
    void stop();                                         // Stop the web server
    bool isRunning() const { return running.load(); }   // Check if server is running
    int getPort() const { return port; }                // Get the server port
};
```

### Key Features:

1. **Thread Safety**: Uses an `atomic<bool>` for the `running` flag to ensure thread-safe server control
2. **Flexible Port Assignment**: Automatically tries multiple ports if the default port (8080) is busy
3. **Integration with Scheduler**: Direct integration with the `MLFQScheduler` to provide real-time data
4. **HTTP Server**: Built-in HTTP server without external dependencies

### Public Methods:

- `WebServer(MLFQScheduler* sched, int port = 8080)` - Constructor that takes a pointer to an MLFQ scheduler instance and an optional port number
- `~WebServer()` - Destructor that ensures the server is properly stopped when the object is destroyed
- `start()` - Starts the HTTP server in a separate thread
- `stop()` - Stops the server and joins the server thread
- `isRunning()` - Returns whether the server is currently running
- `getPort()` - Returns the port number the server is running on

---

## WebServer.cpp - Implementation

The `WebServer.cpp` file implements the web server functionality using POSIX sockets for Linux/macOS/Unix systems.

### Dependencies

```cpp
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
```

### Server Initialization and Startup

The server is implemented as a single-threaded TCP server that creates a background thread to handle incoming connections. When `start()` is called:

1. It creates a TCP socket using `socket(AF_INET, SOCK_STREAM, 0)`
2. Sets socket options to reuse addresses with `SO_REUSEADDR`
3. Binds to all local interfaces on the specified port (with automatic port incrementing if busy)
4. Listens with a backlog of 5 pending connections
5. Starts an infinite loop to accept client connections

The implementation includes automatic port selection - if the default port (8080) is busy, the server will try up to 10 consecutive ports before giving up.

### Thread Management

The server runs in a separate thread to avoid blocking the main application. The `running` atomic flag controls the server loop, and the server thread is properly joined when the server stops.

### Client Request Handling

The `handleClient()` method processes incoming HTTP requests by:

1. Reading the request data from the client socket using `read()`
2. Parsing the request to determine the requested resource by checking the beginning of the request string
3. Serving appropriate content based on the requested path
4. Constructing proper HTTP responses with appropriate headers and content
5. Sending the response back to the client using `send()`

### Static File Serving

The server serves static files from the `web_gui/` directory:

- `GET /` serves the main `index.html` file
- `GET /styles.css` serves the CSS file
- `GET /script.js` serves the JavaScript file

For each file request, the server:
1. Opens the appropriate file from the web_gui directory
2. Reads the file content into a string
3. Constructs an HTTP response with proper headers (Content-Type, Content-Length)
4. Sends the response to the client

### API Endpoints Implementation

The server handles several API endpoints that interact with the MLFQ scheduler:

- `GET /api/status` - Returns scheduler statistics in JSON format
- `GET /api/processes` - Returns process information in JSON format
- `GET /api/queues` - Returns queue information in JSON format
- `POST /api/step` - Executes one step of the scheduler
- `POST /api/reset` - Resets the scheduler
- `POST /api/preset` - Loads preset process configurations
- `POST /api/random` - Generates random processes
- `POST /api/add-process` - Adds a new process
- `POST /api/config` - Updates scheduler configuration

### Error Handling

The server handles several types of errors:

1. Socket creation and binding failures
2. File not found errors (returns 404 responses)
3. Malformed requests (returns appropriate error responses)
4. Invalid parameters in POST requests (uses defaults or clamps to valid ranges)

The server ensures that the socket is properly closed and the connection is terminated even in error conditions.

---

## API Endpoints

The WebServer implements several RESTful API endpoints that allow the web interface to interact with the MLFQ scheduler:

### GET Endpoints

#### GET /api/status
Returns current scheduling statistics and system status.

**Response Format (JSON):**
```json
{
  "time": 42,
  "hasProcesses": true,
  "completedProcesses": 3,
  "totalProcesses": 8,
  "cpuUtilization": 85.7,
  "avgWaitTime": 4.2,
  "avgTurnaroundTime": 12.5,
  "avgResponseTime": 2.1,
  "throughput": 0.15,
  "boostInterval": 20,
  "boostEnabled": true,
  "lastBoostTime": 22,
  "nextBoostIn": 18
}
```

#### GET /api/processes
Returns detailed information about all processes.

**Response Format (JSON):**
```json
{
  "currentRunningPid": 1,
  "processes": [
    {
      "pid": 1,
      "arrival": 0,
      "burst": 20,
      "remaining": 15,
      "queue": 0,
      "completion": 0,
      "status": "Running"
    }
  ]
}
```

#### GET /api/queues
Returns information about all MLFQ priority queues.

**Response Format (JSON):**
```json
{
  "queues": [
    {
      "id": 0,
      "size": 1,
      "quantum": 4,
      "processes": [1]
    }
  ]
}
```

#### GET /api/export-csv
Exports all process information and scheduler metrics in CSV format.

**Response Format (CSV):**
```
PID,Arrival_Time,Burst_Time,Remaining_Time,Completion_Time,Wait_Time,Turnaround_Time,Response_Time,Status,Queue_Level
1,0,20,15,0,0,0,0,Running,0
2,5,12,12,0,0,0,0,Ready,0

SUMMARY METRICS,,,
Current_Time,42
Total_Processes,8
Completed_Processes,3
CPU_Utilization,85.70%
Avg_Wait_Time,4.20
Avg_Turnaround_Time,12.50
Avg_Response_Time,2.10
Throughput,0.15
```

The CSV includes:

1. **Process Data**: Individual process information including PID, arrival time, burst time, remaining time, completion time, wait time, turnaround time, response time, status, and queue level
2. **Summary Metrics**: Overall scheduler statistics including current time, total processes, completed processes, CPU utilization percentage, average wait time, average turnaround time, average response time, and throughput

### POST Endpoints

#### POST /api/step
Executes one time unit of the scheduler.

**Request Body:** None
**Response:** Success confirmation

**Implementation Details:**
The step endpoint implementation creates a complete HTTP response with specific headers:

```cpp
response = "HTTP/1.1 200 OK\r\n";
response += "Content-Type: application/json\r\n";
response += "Access-Control-Allow-Origin: *\r\n";
response += "Content-Length: 15\r\n";
response += "\r\n";
response += "{\"success\":true}";
```

- `HTTP/1.1 200 OK\r\n`: HTTP status line indicating successful request
- `Content-Type: application/json\r\n`: Specifies that the response body contains JSON data
- `Access-Control-Allow-Origin: *\r\n`: CORS header allowing cross-origin requests (important for web browsers)
- `Content-Length: 15\r\n`: Specifies the length of the response body (15 characters for the JSON string)
- `\r\n`: Empty line separating headers from the body (required by HTTP protocol)
- `{"success":true}`: JSON response confirming the step operation succeeded

#### POST /api/reset
Resets the scheduler to its initial state.

**Request Body:** None
**Response:** Success confirmation

#### POST /api/preset
Loads a predefined set of processes based on the set number.

**Request Body:**
```
set=1
```
Where `set` can be a value from 1-8 representing different process configurations.

**Response:** Success confirmation

#### POST /api/random
Generates random processes based on specified parameters.

**Request Body:**
```
count=5&maxArrival=10&minBurst=1&maxBurst=15
```
Parameters:
- `count`: Number of processes to generate (1-50)
- `maxArrival`: Maximum arrival time (0-100)
- `minBurst`: Minimum burst time (1-50)
- `maxBurst`: Maximum burst time (1-100)

**Response:** Success confirmation

#### POST /api/add-process
Adds a new process to the scheduler.

**Request Body:**
```
arrival=0&burst=5
```
Parameters:
- `arrival`: Arrival time
- `burst`: Burst time

**Response:** Success confirmation

#### POST /api/config
Updates scheduler configuration parameters.

**Request Body:**
```
algorithm=rr&boost=true&interval=20&speed=5&numQueues=3&baseQuantum=4&quantumMultiplier=2.0
```
Parameters:
- `algorithm`: Last queue algorithm (rr, sjf, priority)
- `boost`: Whether to enable priority boost (true/false)
- `interval`: Boost interval in time units
- `speed`: Animation speed (1-10)
- `numQueues`: Number of queues (2-5)
- `baseQuantum`: Base time quantum (1-10)
- `quantumMultiplier`: Quantum multiplier (1.0-5.0)

**Response:** Success confirmation

---

## Web GUI Structure

The web GUI is located in the `/web_gui/` directory and consists of several files that work together to create a modern, interactive interface for the MLFQ scheduler:

### index.html
The main HTML file that provides the complete user interface structure for the MLFQ scheduler web interface. Key components include:

- **Header Section**: Title and application branding
- **Control Panel**: Interactive buttons for controlling the simulation:
  - Start (▶️), Pause (⏸️), Step (⏭️), Reset (🔄) controls
  - Preset Processes (📋) and Random Processes (🎲) buttons
  - Add Process (➕), Configuration (⚙️), and Export CSV (📊) buttons
- **Status Panel**: Real-time metrics display:
  - Current time, active processes, completed processes, and CPU utilization
- **Main Content Area**: Two-column layout showing:
  - Process queues visualization with priority levels and time quantums
  - Process details table with PID, arrival, burst, remaining, completion, and status
  - Performance metrics cards (average wait, turnaround, and response times)
- **Modal Dialogs**: Separate sections for:
  - Configuration settings (speed, queues, quantum, algorithms)
  - Add process form (arrival and burst time inputs)
  - Preset process sets (8 different workload configurations)
  - Random process generator (with parameters)
- **Script Inclusion**: Links to the JavaScript file for frontend logic

### styles.css
The CSS stylesheet that provides comprehensive styling and animations for the web interface. Key features include:

- **Modern Design**: Gradient backgrounds using `linear-gradient` from colors #667eea to #764ba2
- **Responsive Layout**: Uses CSS Grid and Flexbox for adaptive layouts on different screen sizes
- **Visual Hierarchy**: Professional color scheme with consistent typography and spacing
- **Queue Visualization**: Styling for priority queues with color-coded borders and hover effects
- **Process Items**: Individual process display with special highlighting for running processes
- **Animation Effects**:
  - Pulse animation for running processes using `@keyframes pulse`
  - Hover effects and smooth CSS transitions
  - Glow animation for currently running processes in the table
- **Modal Dialogs**: Overlay styling with centered content and responsive sizing
- **Interactive Elements**: Button hover effects, table row highlighting, and form element styling
- **Media Queries**: Responsive design that adapts to mobile devices and tablets

### script.js
The JavaScript file implementing the complete frontend logic with the `MLFQWebInterface` class. Key features include:

- **Class Architecture**: The `MLFQWebInterface` class manages the entire web interface state
- **API Communication**: Methods that use the Fetch API to communicate with the backend server
- **Real-time Updates**: Automatic polling every 500ms to keep the UI synchronized with the scheduler
- **Event Management**: Comprehensive event listeners for all UI controls and keyboard events
- **Modal Management**: Functions to show, hide, and handle user input from modal dialogs
- **Data Processing**: Methods to process JSON responses from the backend and update the UI accordingly
- **Visualization Logic**: Functions to update process queues, highlight running processes, and maintain the visual state
- **Error Handling**: Graceful degradation when the backend is unavailable
- **Performance Optimization**: Efficient DOM updates to maintain smooth animations

The JavaScript code follows modern ES6+ standards with class-based architecture, asynchronous programming with async/await, and modular design principles.

### README.md
Documentation file explaining the web interface features, usage instructions, technical implementation details, and customization options. This file provides comprehensive information about the web GUI for developers and users.

---

## Interaction Between Backend and Frontend

The interaction between the C++ backend and the JavaScript frontend follows a client-server architecture with REST API communication:

### Initialization Flow

1. The C++ main program initializes an MLFQScheduler instance with the desired configuration
2. A WebServer instance is created, taking a reference to the scheduler instance
3. The WebServer starts on port 8080 (or an available port), beginning to listen for HTTP requests
4. When a user opens their browser to `http://localhost:8080`, the server serves the static `index.html` file
5. The browser then requests `styles.css` and `script.js`, which the server also provides
6. The JavaScript frontend initializes with the `MLFQWebInterface` class and begins periodic polling

### Communication Protocol

All communication between frontend and backend happens through HTTP requests:

- **Static Content**: Simple GET requests for HTML, CSS, and JS files
- **API Endpoints**: GET requests for data retrieval and POST requests for actions
- **Content-Type Headers**: Proper headers are set for JSON responses and form data
- **CORS Headers**: Cross-origin resource sharing headers are included for browser compatibility

### Real-time Data Synchronization

The frontend maintains synchronization with the backend through:

1. **Status Polling**: Every 500ms, the frontend requests `/api/status` to get current scheduler metrics
2. **Process Data Polling**: Concurrent requests to `/api/processes` and `/api/queues` provide detailed state information
3. **State Mapping**: JavaScript objects map to the internal state of the C++ MLFQScheduler
4. **Visual Updates**: The DOM is updated to reflect the current state of queues, processes, and metrics

### Request Processing Flow

For API endpoints, the flow follows this pattern:

1. JavaScript sends an HTTP request (fetch API) to a specific endpoint
2. The C++ WebServer's `handleClient()` method receives and parses the HTTP request
3. The appropriate scheduler method is called (e.g., `scheduler->step()`)
4. The scheduler modifies its internal state
5. The result is formatted as JSON and sent back as an HTTP response
6. The JavaScript receives the response and updates the UI accordingly

### Data Transformation

The system handles data transformation between C++ and JavaScript:

- **C++ Objects** → **JSON Format** → **JavaScript Objects** → **DOM Elements**
- Complex scheduler data structures are serialized to JSON for transmission
- Process states, queue configurations, and metrics are converted to simple data types suitable for JSON
- JavaScript parses JSON responses and updates the appropriate UI components

### Concurrency Considerations

The system handles concurrent access through:

- **Atomic Operations**: The `running` flag uses atomic operations to prevent race conditions
- **Thread Safety**: The WebServer operates in its own thread, separate from the main program
- **State Consistency**: All scheduler state modifications happen within the same thread, ensuring consistency
- **Polling Rate**: The 500ms polling interval balances responsiveness with performance

### Error Handling and Recovery

- **Network Failures**: The JavaScript code degrades gracefully when the backend is unavailable
- **Invalid Requests**: The C++ server validates request parameters and returns appropriate error responses
- **State Recovery**: After temporary disconnections, the frontend resynchronizes with the current state
- **Process Validation**: Input parameters are validated and clamped to reasonable ranges

This architecture provides a seamless user experience where users can interact with the MLFQ scheduler through a modern, responsive web interface while leveraging the computational performance and reliability of the C++ backend implementation.

---