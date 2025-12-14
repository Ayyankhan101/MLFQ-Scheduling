# MLFQ Scheduler - Technical Documentation

## Architecture Overview

### System Components

```
┌─────────────────────────────────────────────────────────┐
│                    Main Program                         │
│                     (main.cpp)                          │
└────────────────┬────────────────────────────────────────┘
                 │
        ┌─────────┴─────────┐
        │                   │
   ┌────▼────┐        ┌─────▼─────┐
   │TerminalUI│       │SchedulerConfig│
   │         │        │             │
   │ - Style │        │ - Params    │
   │ - Clear │        │ - Presets   │
   │ - Header│        │ - Validation│
   └─────────┘        └───────────┘
        │                   │
        └─────────┬─────────┘
                  │
        ┌─────────┴─────────┐
        │                   │
   ┌────▼────────┐  ┌───────▼───────┐
   │  Visualizer │  │  MLFQScheduler│
   │             │  │               │
   │ - Display   │  │ - MLFQ Logic  │
   │ - Stats     │  │ - Queues      │
   │ - Gantt     │  │ - Timing      │
   └─────────────┘  └───────┬───────┘
                            │
                     ┌──────▼──────┐
                     │ProcessQueue │
                     │             │
                     │ - FIFO      │
                     │ - Round RR  │
                     └─────────────┘
                            │
                     ┌──────▼──────┐
                     │   Process   │
                     │             │
                     │ - PCB Data  │
                     │ - Metrics   │
                     │ - Exec      │
                     └─────────────┘
```

### Web Architecture

```
┌─────────────────┐    HTTP     ┌─────────────────┐
│   C++ Backend   │◄──────────►│  Web Frontend   │
│                 │   Port 8080 │                 │
│ ┌─────────────┐ │             │ ┌─────────────┐ │
│ │ WebServer   │ │             │ │ JavaScript  │ │
│ │ - Static    │ │             │ │ - Frontend  │ │
│ │ - Files     │ │             │ │ - Logic     │ │
│ └─────────────┘ │             │ └─────────────┘ │
│                 │             │                 │
│ ┌─────────────┐ │             │ ┌─────────────┐ │
│ │ MLFQ Core   │ │             │ │ Visualization│ │
│ │ Algorithm   │ │             │ │ - Real-time │ │
│ │             │ │             │ │ - Interactive│ │
│ └─────────────┘ │             │ └─────────────┘ │
└─────────────────┘             └─────────────────┘
```

## Core Algorithm Implementation

### MLFQ Scheduler Class

```cpp
class MLFQScheduler {
private:
    vector<ProcessQueue> readyQueues;           // Priority queues
    vector<shared_ptr<Process>> allProcesses;   // All processes
    vector<shared_ptr<Process>> completedProcesses;
    shared_ptr<Process> currentProcess;         // Currently running

    int currentTime;                            // System clock
    int boostTimer;                            // Aging timer
    int boostInterval;                         // Boost frequency
    int numQueues;                             // Number of queues
    int pidCounter;                            // Process ID counter

    SchedulerConfig config;                    // Configuration
    LastQueueAlgorithm lastQueueAlgorithm;     // Last queue algorithm

    vector<ExecutionRecord> executionLog;      // Gantt chart data

    // Throughput matrix tracking - stores throughput at different time intervals
    vector<pair<int, double>> throughputMatrix; // Pair of (time, throughput_at_that_time)
    int throughputInterval;  // Time interval at which to record throughput (default 10)

    // Variables to track better throughput calculation
    int firstArrivalTime;  // Time of the first process arrival
    bool firstArrivalRecorded;  // Flag to ensure first arrival is recorded only once

public:
    // Constructors
    MLFQScheduler(int queues, int boost);              // Legacy
    MLFQScheduler(const SchedulerConfig& cfg);         // Configuration-based
    
    // Core scheduling methods
    void addProcess(int arrivalTime, int burstTime);
    void addProcess(shared_ptr<Process> process);
    bool step();                                       // Execute one time unit
    void run();                                        // Run to completion
    
    // Algorithm configuration
    void setLastQueueAlgorithm(LastQueueAlgorithm alg);
    void setBoostInterval(int interval);
    
    // Statistics and metrics
    SchedulerStats getStats() const;
    void displayStats() const;
    void exportToCSV(const string& filename) const;

    // Throughput Matrix methods
    const vector<pair<int, double>>& getThroughputMatrix() const;
    void setThroughputInterval(int interval);
    int getThroughputInterval() const;
    void updateThroughputMatrix();

    // State access
    const vector<ProcessQueue>& getQueues() const;
    shared_ptr<Process> getCurrentProcess() const;
    int getCurrentTime() const;
};
```

### Process Control Block

```cpp
enum class ProcessState {
    NEW,         // Just created
    READY,       // In ready queue
    RUNNING,     // Currently executing
    TERMINATED   // Completed execution
};

class Process {
private:
    int pid;                    // Process ID
    int arrivalTime;           // When process arrives
    int burstTime;             // Total CPU time needed
    int remainingTime;         // CPU time left
    int completionTime;        // When process finished
    int waitTime;              // Time spent waiting
    int turnaroundTime;        // Total time in system
    int responseTime;          // Time to first CPU access
    int firstResponseTime;     // When first got CPU
    ProcessState state;        // Current state
    int currentQueue;          // Which queue process is in
    int quantumUsed;           // Time used in current quantum
    
public:
    // Constructor
    Process(int id, int arrival, int burst);
    
    // Execution
    bool execute(int timeSlice);           // Run for time slice
    void setState(ProcessState newState);
    
    // Metrics calculation
    void calculateMetrics();
    
    // Getters
    int getPid() const { return pid; }
    int getArrivalTime() const { return arrivalTime; }
    int getBurstTime() const { return burstTime; }
    int getRemainingTime() const { return remainingTime; }
    ProcessState getState() const { return state; }
    // ... other getters
};
```

### Queue Management

```cpp
class ProcessQueue {
private:
    queue<shared_ptr<Process>> processes;
    int queueLevel;            // Priority level (0 = highest)
    int timeQuantum;           // Time slice for this queue
    
public:
    ProcessQueue(int level, int quantum);
    
    void enqueue(shared_ptr<Process> process);
    shared_ptr<Process> dequeue();
    shared_ptr<Process> front() const;
    
    bool isEmpty() const;
    size_t size() const;
    int getTimeQuantum() const;
    int getQueueLevel() const;
    
    // Round-robin support
    void roundRobin();
    
    // Queue-specific algorithms
    shared_ptr<Process> shortestJobFirst();
    shared_ptr<Process> priorityScheduling();
};
```

## Algorithm Details

### MLFQ Rules Implementation

1. **Rule 1**: If Priority(A) > Priority(B), A runs (B doesn't)
2. **Rule 2**: If Priority(A) = Priority(B), A & B run in round-robin
3. **Rule 3**: When a job enters the system, it is placed at the highest priority
4. **Rule 4a**: If a job uses up its time slice, its priority is reduced
5. **Rule 4b**: If a job gives up the CPU before the time slice is up, it stays at the same priority level
6. **Rule 5**: After some time period S, move all the jobs to the topmost queue (priority boost)

### Time Quantum Progression

```cpp
// Default quantum progression: 1, 2, 4, 8, 16, ...
int SchedulerConfig::getQuantumForQueue(int queueLevel) const {
    if (queueLevel >= quantums.size()) {
        // Use exponential progression for higher queues
        return static_cast<int>(baseQuantum * pow(quantumMultiplier, queueLevel));
    }
    return quantums[queueLevel];
}
```

### Priority Boost (Aging)

```cpp
void MLFQScheduler::priorityBoost() {
    if (boostInterval <= 0) return;  // Boost disabled
    
    boostTimer++;
    if (boostTimer >= boostInterval) {
        // Move all processes to highest priority queue
        for (int i = 1; i < numQueues; i++) {
            while (!readyQueues[i].isEmpty()) {
                auto process = readyQueues[i].dequeue();
                process->setCurrentQueue(0);
                readyQueues[0].enqueue(process);
            }
        }
        boostTimer = 0;  // Reset timer
    }
}
```

### Last Queue Algorithms

The lowest priority queue can use different scheduling algorithms:

#### Round Robin (Default)
```cpp
shared_ptr<Process> ProcessQueue::roundRobin() {
    if (isEmpty()) return nullptr;
    
    auto process = dequeue();
    // Process will be re-enqueued after execution
    return process;
}
```

#### Shortest Job First
```cpp
shared_ptr<Process> ProcessQueue::shortestJobFirst() {
    if (isEmpty()) return nullptr;
    
    // Find process with shortest remaining time
    shared_ptr<Process> shortest = nullptr;
    int minTime = INT_MAX;
    
    // Implementation details...
    return shortest;
}
```

#### Priority Scheduling with Aging
```cpp
shared_ptr<Process> ProcessQueue::priorityScheduling() {
    if (isEmpty()) return nullptr;
    
    // Calculate dynamic priorities based on wait time
    // Longer wait time = higher priority (aging)
    // Implementation details...
    return highestPriorityProcess;
}
```

## Performance Metrics

### Calculation Formulas

```cpp
struct SchedulerStats {
    double avgWaitTime;        // Average time processes wait
    double avgTurnaroundTime;  // Average total time in system
    double avgResponseTime;    // Average time to first CPU access
    double cpuUtilization;     // Percentage of CPU busy time
    double throughput;         // Processes completed per time unit
    int totalProcesses;        // Total processes handled
    int completedProcesses;    // Processes that finished
    int currentTime;           // Current system time
};

// Calculations
void MLFQScheduler::calculateStats() {
    double totalWait = 0, totalTurnaround = 0, totalResponse = 0;
    int completed = completedProcesses.size();
    
    for (const auto& process : completedProcesses) {
        totalWait += process->getWaitTime();
        totalTurnaround += process->getTurnaroundTime();
        totalResponse += process->getResponseTime();
    }
    
    stats.avgWaitTime = completed > 0 ? totalWait / completed : 0;
    stats.avgTurnaroundTime = completed > 0 ? totalTurnaround / completed : 0;
    stats.avgResponseTime = completed > 0 ? totalResponse / completed : 0;
    
    // CPU Utilization = (Total CPU Time) / (Total Elapsed Time)
    int totalCpuTime = 0;
    for (const auto& process : allProcesses) {
        totalCpuTime += process->getBurstTime();
    }
    stats.cpuUtilization = currentTime > 0 ? 
        (double)totalCpuTime / currentTime * 100 : 0;
}
```

### Metric Definitions

- **Wait Time**: Time spent in ready queues (not including execution time)
  ```
  Wait Time = Turnaround Time - Burst Time
  ```

- **Turnaround Time**: Total time from arrival to completion
  ```
  Turnaround Time = Completion Time - Arrival Time
  ```

- **Response Time**: Time from arrival to first CPU access
  ```
  Response Time = First CPU Access Time - Arrival Time
  ```

- **Throughput**: Number of processes completed per time unit
  ```
  Throughput = Completed Processes / (Current Time - First Arrival Time)
  ```

- **CPU Utilization**: Percentage of time CPU is busy
  ```
  CPU Utilization = (Total CPU Time / Total Elapsed Time) × 100
  ```

## Configuration System

### SchedulerConfig Class

```cpp
class SchedulerConfig {
public:
    // Core parameters
    int numQueues = 4;              // Number of priority queues
    int boostInterval = 50;         // Priority boost frequency
    int baseQuantum = 1;            // Base time quantum
    double quantumMultiplier = 2.0; // Quantum progression factor
    
    // Queue-specific quantums
    vector<int> quantums = {1, 2, 4, 8};
    
    // Algorithm selection
    LastQueueAlgorithm lastQueueAlgorithm = LastQueueAlgorithm::ROUND_ROBIN;
    
    // Presets
    static SchedulerConfig getDefaultConfig();
    static SchedulerConfig getInteractiveConfig();
    static SchedulerConfig getBatchConfig();
    static SchedulerConfig getRealtimeConfig();
    
    // Validation
    bool isValid() const;
    void validate();
    
    // Display
    void displayQuantums() const;
    void displayConfig() const;
};
```

### Configuration Presets

#### Default Configuration
- 4 queues with quantums [1, 2, 4, 8]
- Priority boost every 50 time units
- Round-robin in last queue

#### Interactive Configuration
- Shorter quantums for responsiveness
- More frequent priority boosts
- Optimized for interactive processes

#### Batch Configuration
- Longer quantums for throughput
- Less frequent priority boosts
- Optimized for CPU-intensive tasks

#### Real-time Configuration
- Very short quantums
- Frequent priority boosts
- Strict timing requirements

## Web Interface Implementation

### JavaScript MLFQ Engine

The web interface includes a complete JavaScript implementation of the MLFQ algorithm that mirrors the C++ version:

```javascript
class MLFQWebInterface {
    constructor() {
        this.queues = [];
        this.processes = [];
        this.currentTime = 0;
        this.boostTimer = 0;
        this.config = {
            numQueues: 4,
            quantums: [1, 2, 4, 8],
            boostInterval: 50,
            lastQueueAlgorithm: 'roundRobin'
        };
        this.initializeQueues();
    }
    
    step() {
        // Identical logic to C++ implementation
        this.handleArrivals();
        this.executeCurrentProcess();
        this.updateMetrics();
        this.checkPriorityBoost();
        this.updateDisplay();
    }
    
    // ... other methods mirror C++ implementation
}
```

### Real-time Visualization

The web interface provides:

1. **Queue Visualization**: Shows individual process IDs in each queue
2. **Process Table**: Real-time updates of process states and metrics
3. **Performance Dashboard**: Live calculation of scheduling metrics
4. **Interactive Controls**: Start, pause, step, reset functionality
5. **Configuration Panel**: Adjust parameters and see immediate effects

### Synchronization with C++ Backend

Both implementations use identical:
- Algorithm logic and decision-making
- Time quantum progression
- Priority boost mechanisms
- Performance metric calculations
- Process state management

This ensures educational consistency between interfaces.

## Testing and Validation

### Unit Tests

```cpp
// Test process creation and basic operations
void testProcessCreation();
void testQueueOperations();
void testSchedulerBasics();

// Test algorithm correctness
void testMLFQRules();
void testPriorityBoost();
void testLastQueueAlgorithms();

// Test performance metrics
void testMetricCalculations();
void testEdgeCases();
```

### Integration Tests

- Cross-platform compatibility (Linux, macOS, Windows)
- Web interface consistency with C++ backend
- Performance testing with large process sets
- Algorithm comparison validation

### Validation Methods

1. **Textbook Verification**: Results match standard MLFQ examples
2. **Cross-Implementation Consistency**: C++ and JavaScript produce identical results
3. **Edge Case Testing**: Handle empty queues, single processes, etc.
4. **Performance Validation**: Metrics calculations are mathematically correct

## Build System and Dependencies

### CMake Configuration

```cmake
cmake_minimum_required(VERSION 3.10)
project(MLFQ_Scheduler VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional FLTK support
find_package(PkgConfig REQUIRED)
pkg_check_modules(FLTK QUIET fltk1.3 fltk)

# Core sources
set(CORE_SOURCES
    src/Process.cpp
    src/Queue.cpp
    src/MLFQScheduler.cpp
    src/Visualizer.cpp
    src/WebServer.cpp
)

# Conditional FLTK compilation
if(FLTK_FOUND)
    add_compile_definitions(FLTK_AVAILABLE)
    set(ALL_SOURCES ${CORE_SOURCES} src/FLTKVisualizer.cpp src/main.cpp)
    add_executable(mlfq_scheduler ${ALL_SOURCES})
    target_link_libraries(mlfq_scheduler ${FLTK_LIBRARIES})
else()
    add_executable(mlfq_scheduler ${CORE_SOURCES} src/main.cpp)
endif()

# Test executable
add_executable(test_scheduler ${CORE_SOURCES} tests/test_scheduler.cpp)
```

### Dependencies

#### Required
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10+
- Standard C++ library

#### Optional
- FLTK 1.3+ (for GUI interface)
- Modern web browser (for web interface)

#### No External Dependencies
- Web server is built-in (no Apache/Nginx needed)
- All algorithms implemented from scratch
- Cross-platform socket programming

## Performance Characteristics

### Time Complexity
- **Process Addition**: O(1)
- **Single Step Execution**: O(n) where n is number of processes
- **Queue Operations**: O(1) for enqueue/dequeue
- **Priority Boost**: O(n) where n is total processes
- **Shortest Job First**: O(n) for last queue selection

### Space Complexity
- **Process Storage**: O(n) where n is number of processes
- **Queue Storage**: O(n) distributed across priority levels
- **Execution Log**: O(t) where t is total execution time
- **Statistics**: O(1) for metric storage

### Scalability
- Handles hundreds of processes efficiently
- Real-time visualization up to ~50 processes
- Memory usage scales linearly with process count
- Web interface optimized for educational use (not production scale)

## Future Enhancements

### Potential Improvements
1. **Multi-core MLFQ**: Simulate multiple CPU cores
2. **Process Migration**: Dynamic load balancing between cores
3. **Advanced Metrics**: Cache hit rates, context switch overhead
4. **Mobile Interface**: Responsive design for tablets/phones
5. **Export Capabilities**: PDF reports, CSV data export
6. **Animation Controls**: Slow motion, replay functionality

### Architecture Extensions
1. **Plugin System**: Custom scheduling algorithms
2. **Network Simulation**: Distributed MLFQ across nodes
3. **Real-time Integration**: Interface with actual OS scheduler
4. **Machine Learning**: Adaptive parameter tuning

This technical documentation provides a comprehensive overview of the MLFQ scheduler implementation, covering both the algorithmic details and the software engineering aspects that make it an effective educational tool.## Core Classes

### 1. Process Class

**Purpose**: Represents a Process Control Block (PCB)

**Key Attributes:**
```cpp
int pid;                    // Process identifier
int priority;               // Current queue level (0 = highest)
int arrivalTime;            // When process enters system
int burstTime;              // Total CPU time needed
int remainingTime;          // CPU time still needed
int cpuTimeUsed;            // CPU time consumed so far
int waitTime;               // Time spent waiting
int responseTime;           // Time from arrival to first execution
int completionTime;         // When process completed execution
ProcessState state;         // Current state (NEW, READY, RUNNING, etc.)
```

**Key Methods:**
- `execute(timeSlice, currentTime)`: Simulates CPU execution
- `calculateMetrics()`: Computes wait time, turnaround time, etc.
- `incrementWaitTime()`: Called when process is waiting
- `resetToHighestPriority()`: Used for aging mechanism
- `setFirstExecutionTime(time)`: Records first execution time

**State Machine:**
```
NEW → READY → RUNNING → TERMINATED
        ↑         ↓
        └─────────┘
      (Preemption)
```

### 2. ProcessQueue Class

**Purpose**: Manages a single priority level queue

**Implementation**: Uses `std::deque` for O(1) enqueue/dequeue

**Key Methods:**
- `enqueue(process)`: Add process to back of queue
- `dequeue()`: Remove and return front process
- `peek()`: View front process without removing
- `removeProcess(pid)`: Remove specific process (for termination)
- `size()`: Get number of processes in queue
- `isEmpty()`: Check if queue is empty

**Design Decision**: Using deque instead of queue for iteration capability (needed for visualization)

### 3. MLFQScheduler Class

**Purpose**: Implements core MLFQ scheduling algorithm

**Key Attributes:**
```cpp
vector<ProcessQueue> readyQueues;           // Multiple priority levels
vector<shared_ptr<Process>> allProcesses;   // All processes in system
vector<shared_ptr<Process>> completedProcesses; // Completed processes
shared_ptr<Process> currentProcess;         // Currently executing process
int currentTime;                            // System clock
int boostTimer;                             // Timer for aging
int numQueues;                              // Number of priority levels
int boostInterval;                          // Time between priority boosts
vector<int> timeQuantums;                   // Quantum for each queue level
SchedulerConfig config;                     // Configuration parameters
LastQueueAlgorithm lastQueueAlg;            // Algorithm for last queue
```

**Scheduling Algorithm:**

```cpp
void MLFQScheduler::step() {
    1. Check for new arrivals
    2. Check if priority boost needed (Rule 4)
    3. If no current process, select next from highest non-empty queue
    4. Execute current process for 1 time unit
    5. Update wait times for all waiting processes
    6. Check if process completed or quantum expired
    7. Apply MLFQ rules:
       - Rule 1: Priority-based execution (higher priority first)
       - Rule 2: Round-robin for equal priority (within same queue)
       - Rule 3: Demote process if time quantum used up
       - Rule 4: Priority boost at intervals (implemented in step 2)
    8. Move process to appropriate queue or terminate
    9. Increment system time
}
```

**Queue Selection Logic:**
```cpp
// Rule 1: Higher priority first
for (int i = 0; i < numQueues; i++)
{
    if (!readyQueues[i].isEmpty())
    {
        // If it's the last queue, use the configured algorithm
        if (i == numQueues - 1 && lastQueueAlg != LastQueueAlgorithm::ROUND_ROBIN) {
            return selectNextProcessForLastQueue(i);
        } else {
            return readyQueues[i].dequeue();
        }
    }
}
```

**Priority Adjustment:**
```cpp
// Rule 3: Move to lower priority when quantum expires
void moveToNextQueue(shared_ptr<Process> p)
{
    int nextPriority = min(p->priority + 1, numQueues - 1);
    readyQueues[nextPriority].enqueue(p);
    p->priority = nextPriority;
}
```

**Aging Mechanism:**
```cpp
// Rule 4: Boost all processes periodically
void boostAllProcesses()
{
    for (int i = 1; i < numQueues; i++) // Start from queue 1 (not 0)
    {
        while (!readyQueues[i].isEmpty()) {
            auto process = readyQueues[i].dequeue();
            process->resetToHighestPriority();
            readyQueues[0].enqueue(process);
        }
    }
}
```

### 4. Visualizer Class

**Purpose**: Provides terminal-based visualization

**Display Components:**
1. **Current Execution Panel**: Shows running process with progress bar
2. **Queue Status Panel**: Displays all queues and their contents
3. **Statistics Panel**: Shows metrics (wait time, turnaround time, etc.)
4. **Gantt Chart**: Timeline of completed processes
5. **Process Table**: Shows all processes with arrival and burst times

**Key Methods:**
- `displayQueues()`: Shows all priority queues
- `displayCurrentProcess()`: Shows executing process
- `displayStats()`: Shows scheduling metrics
- `displayGanttChart()`: Shows process timeline
- `displayProcessTable()`: Shows initial process table
- `displayAll()`: Composite display of all panels
- `displayAllWithProcessInfo()`: Shows all displays with process information

### 5. FLTKVisualizer Class (Optional)

**Purpose**: Provides GUI visualization using FLTK library

**Key Components:**
- `Fl_Window`: Main application window
- `Fl_Box`: For displaying text and status
- `Fl_Slider`: For controlling visualization speed
- `Fl_Button`: For control buttons
- `Fl_Timer`: For animation timing

**Key Methods:**
- `run()`: Start the GUI event loop
- `updateDisplay()`: Refresh the visualization
- `onStep()`: Handle single step execution
- `onAuto()`: Start automatic execution
- `onStop()`: Stop execution
- `onSpeedChange()`: Update animation speed

**Features:**
- Real-time process monitoring
- Interactive controls
- Cross-platform compatibility
- Modern interface design

### 6. SchedulerConfig Class

**Purpose**: Manages scheduler configuration parameters

**Key Attributes:**
```cpp
int numQueues;              // Number of priority levels (2-5)
int baseQuantum;            // Base time quantum (2-10 ms)
double quantumMultiplier;   // Growth factor for queue quantums (1.0-5.0x)
int boostInterval;          // Priority boost interval (20-500 ms)
int animationDelay;         // Animation speed (50-2000 ms)
int numProcesses;           // For random generation (1-20)
int maxArrivalTime;         // Max arrival time (0-100 ms)
int minBurstTime;           // Min burst time (1-100 ms)
int maxBurstTime;           // Max burst time (1-100 ms)
```

**Key Methods:**
- `validate()`: Validate parameter ranges
- `display()`: Show current configuration
- `displayQuantums()`: Show calculated time quantums for each queue

**Presets:**
- `STANDARD_MLFQ`: 3 queues, 4ms base quantum, 2.0x multiplier, 100ms boost
- `FINE_GRAINED`: 5 queues, 2ms base quantum, 1.5x multiplier, 50ms boost
- `COARSE_GRAINED`: 2 queues, 8ms base quantum, 3.0x multiplier, 200ms boost
- `AGGRESSIVE_ANTI_STARVATION`: 3 queues, 4ms base quantum, 2.0x multiplier, 30ms boost

### 7. TerminalUI Class

**Purpose**: Provides terminal UI utilities and formatting

### 8. WebServer Class

**Purpose**: Built-in HTTP server to serve the web-based GUI interface

**Key Attributes:**
```cpp
MLFQScheduler* scheduler;  // Pointer to the scheduler instance
atomic<bool> running;      // Server running state
thread serverThread;       // Server execution thread
int port;                  // Server port (default 8080)
```

**Key Methods:**
- `start()`: Start the HTTP server in a separate thread
- `stop()`: Stop the HTTP server and join the thread
- `handleClient(int clientSocket)`: Handle individual HTTP requests
- `isRunning()`: Check if server is currently running
- `getPort()`: Get the current server port

**Implementation Details:**
- Uses standard POSIX sockets for cross-platform compatibility
- Serves static files (HTML, CSS, JavaScript) from the web_gui directory
- Supports multiple port selection if default is busy (tries ports from requested to requested+9)
- Handles GET requests for index.html, styles.css, and script.js
- Reuses port with SO_REUSEADDR to prevent binding issues

**HTTP Request Handling:**
```cpp
void WebServer::handleClient(int clientSocket) {
    // Read HTTP request
    char buffer[4096] = {0};
    read(clientSocket, buffer, sizeof(buffer));
    string request(buffer);

    // Route based on requested resource
    if (request.find("GET / ") != string::npos) {
        // Serve index.html
    } else if (request.find("GET /styles.css") != string::npos) {
        // Serve styles.css
    } else if (request.find("GET /script.js") != string::npos) {
        // Serve script.js (contains the MLFQ implementation in JS)
    }
}
```

**Thread Safety:**
- Uses atomic<bool> for thread-safe running flag
- Server runs in dedicated thread to not block main application
- No shared mutable state between server and scheduler (the JS runs independently)

### 9. TerminalUI Class

**Purpose**: Provides terminal UI utilities and formatting

**Key Methods:**
- `Style::header()`: Formatted header text
- `Style::success()`: Success messages
- `Style::error()`: Error messages
- `Style::warning()`: Warning messages
- `Style::info()`: Informational text
- `Style::highlight()`: Highlighted text
- `clearScreen()`: Clear the terminal screen
- `drawHeader()`: Draw a formatted header with separator
- `drawSeparator()`: Draw a separator line

### 8. LastQueueAlgorithm Enum

**Purpose**: Defines algorithm options for the last queue

**Values:**
```cpp
enum class LastQueueAlgorithm
{
    ROUND_ROBIN,
    SHORTEST_JOB_FIRST,
    PRIORITY_SCHEDULING
};
```

## Algorithm Implementation Details

### Time Quantum Management

Each queue has exponentially increasing time quantum:
```cpp
for (int i = 0; i < numQueues; i++) {
    timeQuantums.push_back(baseQuantum * pow(quantumMultiplier, i));
}
```

**Quantum Calculation Example:**
- Base Quantum: 4 ms, Multiplier: 2.0
- Queue 0: 4 ms
- Queue 1: 8 ms
- Queue 2: 16 ms
- Queue 3: 32 ms (if exists)

**Rationale:**
- High priority: Short quantum → good interactivity
- Low priority: Long quantum → reduced context switching overhead

### Context Switching

Simplified model:
- No explicit context switch overhead
- Instantaneous switching
- Can be extended to add overhead time

### Metrics Calculation

**Wait Time:**
```cpp
waitTime = completionTime - arrivalTime - burstTime
```

**Turnaround Time:**
```cpp
turnaroundTime = completionTime - arrivalTime
```

**Response Time:**
```cpp
responseTime = firstExecutionTime - arrivalTime
```

**CPU Utilization:**
```cpp
cpuUtilization = (totalBurstTime / totalTime) * 100
```

## Data Structures

### Memory Management

Uses `std::shared_ptr<Process>` for:
- Automatic memory management
- Safe sharing between scheduler and queues
- Prevention of memory leaks

### Queue Implementation

```cpp
std::deque<std::shared_ptr<Process>>
```

**Advantages:**
- O(1) push_back and pop_front
- Random access for visualization
- Iterators for displaying queue contents

## Performance Characteristics

### Time Complexity

- `addProcess()`: O(1)
- `selectNextProcess()`: O(Q) where Q = number of queues
- `step()`: O(N) where N = number of processes (for wait time updates)
- `boostAllProcesses()`: O(N)
- `selectNextProcessForLastQueue()` (SJF/Priority): O(N) where N = processes in last queue

### Space Complexity

- O(N) for storing N processes
- O(Q) for Q queues
- O(E) for execution log with E events

## Configuration

### Tunable Parameters

```cpp
// Using SchedulerConfig class
SchedulerConfig config;
config.numQueues = 3;
config.baseQuantum = 4;
config.quantumMultiplier = 2.0;
config.boostInterval = 100;
config.animationDelay = 200;
MLFQScheduler scheduler(config);
```

### Queue Parameters

Time quantums are calculated automatically:
```cpp
// In MLFQScheduler constructor
for (int i = 0; i < config.numQueues; i++) {
    timeQuantums.push_back(config.baseQuantum * pow(config.quantumMultiplier, i));
}
```

## Extension Points

### Adding I/O Operations

```cpp
struct IORequest
{
    int pid;
    int duration;
    int startTime;
};

class MLFQScheduler
{
    std::queue<IORequest> ioQueue;
    void handleIO();
};
```

### Multiple CPU Cores

```cpp
class MLFQScheduler
{
    std::vector<shared_ptr<Process>> runningProcesses;
    int numCPUs;

    void scheduleMultiCore();
};
```

### Context Switch Overhead

```cpp
class MLFQScheduler
{
    int contextSwitchTime = 1;  // 1 time unit overhead

    void contextSwitch()
    {
        currentTime += contextSwitchTime;
    }
};
```

## Testing Strategy

### Unit Tests

1. **Process Tests**: Creation, execution, metrics
2. **Queue Tests**: Enqueue, dequeue, FIFO behavior
3. **Scheduler Tests**: Basic scheduling, completeness
4. **Metrics Tests**: Calculation accuracy
5. **Configuration Tests**: Parameter validation

### Integration Tests

1. Run with known process sets
2. Verify scheduling order
3. Check metric values against manual calculations
4. Test edge cases (empty queues, single process)
5. Test algorithm switching functionality

### Performance Tests

1. Large number of processes (100+)
2. Long simulation times
3. Visualization rendering speed
4. Algorithm switching performance

## Debugging Tips

### Common Issues

1. **Process never completes**: Check remaining time updates
2. **Incorrect metrics**: Verify time tracking
3. **Starvation**: Check boost mechanism
4. **Queue corruption**: Check shared_ptr usage
5. **Algorithm switching**: Verify proper state transition

### Debug Output

Add debug prints in `MLFQScheduler::step()`:
```cpp
std::cout << "Time: " << currentTime
          << " Running: " << currentProcess->getPid()
          << " Queue: " << currentProcess->getPriority() << "\n";
```

## FLTK Integration

### Conditional Compilation
The GUI functionality is implemented using conditional compilation:

```cpp
#ifdef FLTK_AVAILABLE
#include "FLTKVisualizer.h"
// GUI code here
#else
// Terminal-only fallback
#endif
```

### Build Process
CMake attempts to find FLTK libraries automatically:
- Uses pkg-config first
- Falls back to find_path/find_library
- Adds FLTK_AVAILABLE definition if found

## Results Export

### CSV Export
The system supports exporting scheduling results to CSV format:
- Process details (PID, arrival, burst, completion times)
- Individual metrics (turnaround, wait, response times)
- Summary statistics
- File named with timestamp

## Algorithm Comparison

### Last Queue Algorithms
The system supports different algorithms for the last queue:
1. **Round Robin**: Standard MLFQ behavior
2. **Shortest Job First**: Process with shortest remaining time executes next
3. **Priority Scheduling**: Process with highest priority (shortest burst) executes next

## JavaScript Web Interface Implementation

### MLFQWebInterface Class

**Purpose**: Complete client-side MLFQ scheduler implementation that runs in the browser

**Key Attributes:**
```javascript
this.processes = [];  // Array of process objects with arrivalTime, burstTime, etc.
this.queues = [       // Array of queue objects, each with id, name, quantum, and processes
    { id: 0, name: 'Queue 0 (Highest Priority)', quantum: 1, processes: [] },
    { id: 1, name: 'Queue 1', quantum: 2, processes: [] },
    { id: 2, name: 'Queue 2', quantum: 4, processes: [] },
    { id: 3, name: 'Queue 3 (Lowest Priority)', quantum: 8, processes: [] }
];
this.currentTime = 0;          // Current simulation time
this.isRunning = false;        // Simulation state
this.speed = 5;                // Animation speed multiplier
this.intervalId = null;        // Timer ID for automatic mode
this.lastQueueAlgorithm = 'rr'; // Algorithm for last queue (rr, sjf, fcfs, priority)
this.priorityBoost = true;     // Enable/disable priority boost
this.boostInterval = 20;       // Boost interval in time units
this.currentQuantumTime = 0;   // Track quantum usage for current process
this.currentRunningProcess = null; // Track currently running process
```

**Key Methods:**
- `simulationStep()`: Execute one time unit of the MLFQ algorithm
- `startSimulation()`: Start automatic simulation with specified speed
- `pauseSimulation()`: Pause the simulation
- `resetSimulation()`: Reset all processes and queues to initial state
- `renderQueues()`: Update HTML visualization of queues
- `renderProcessTable()`: Update HTML table showing process information
- `updateMetrics()`: Calculate and display performance metrics
- `insertIntoLastQueue(process, queueIndex)`: Insert process using configured algorithm for last queue
- `performPriorityBoost()`: Move all processes from lower queues to highest priority

**MLFQ Algorithm Implementation:**
The JavaScript implementation mirrors the C++ algorithm:
1. Check for priority boost at boostInterval
2. Check for new arrivals at current time
3. Execute processes in order of queue priority (highest first)
4. Track quantum usage and demote processes when quantum expires
5. Handle process completion and termination

**Last Queue Algorithm Support:**
The JavaScript implementation supports the same three algorithms for the last queue:
- Round Robin (default): Processes added to end of queue
- Shortest Job First: Inserted by remaining time (shortest first)
- First Come First Serve: Inserted by arrival time (earliest first)
- Priority Scheduling: Inserted by wait time (longest waiting first)

**Performance Metrics Calculation:**
- Average Wait Time: Total wait time / completed processes
- Average Turnaround Time: Total turnaround time / completed processes
- CPU Utilization: (Total burst time / current time) * 100
- Throughput: Completed processes / current time

## Configuration Consistency Across All Options

### Implementation Changes:
The system has been enhanced to ensure that user-configured parameters, especially the boost interval and other scheduler settings, are now consistently applied across all execution modes including the algorithm comparison tool:

1. **compareLastQueueAlgorithms() function** now accepts a SchedulerConfig parameter
2. The function uses the passed configuration instead of hardcoded default values (3 queues, 100ms boost)
3. All three comparison runs use identical configuration parameters ensuring fair comparisons
4. The configuration includes all relevant settings: queues count, boost interval, base quantum, quantum multiplier, etc.

### Function Signature Change:
```cpp
// Before: Configuration ignored in comparison
void compareLastQueueAlgorithms();

// After: Uses current configuration for fair comparison
void compareLastQueueAlgorithms(const SchedulerConfig& config);
```

### Integration Points:
- Main menu option 10 (FLTK) / 8 (no FLTK) now passes current config to comparison function
- All three algorithms (RR, SJF, Priority) run with identical configuration parameters
- Temporary schedulers in example loading maintain configuration consistency
- Performance metrics are calculated using the same parameters across all algorithms

### Benefits:
- Algorithm comparisons now reflect performance under user's specific configuration
- Results are more relevant to the user's intended use case
- Configuration changes made via options 5-8 are properly applied in comparisons
- Consistent behavior across all menu options and execution modes

## C++-JavaScript Integration

### Architecture:
1. C++ WebServer serves static files (HTML, CSS, JS) to browser
2. JavaScript runs independently in browser, implementing full MLFQ algorithm
3. No continuous communication between C++ and JavaScript during simulation
4. Both implementations follow identical algorithm logic for consistency

### File Serving Process:
```cpp
void WebServer::handleClient(int clientSocket) {
    // When browser requests /script.js:
    if (request.find("GET /script.js") != string::npos) {
        ifstream file("../web_gui/script.js");
        if (file.is_open()) {
            // Send JavaScript file to browser
        }
    }
}
```
