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

## Core Classes

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
