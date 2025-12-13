# MLFQ Scheduler - A Complete Implementation Journey

## Introduction

Hey there! Welcome to my MLFQ (Multilevel Feedback Queue) scheduler project. This has been quite the journey - what started as a simple class assignment turned into something I'm genuinely proud of. I've spent countless hours making sure this implementation is not just functional, but actually useful for learning how operating systems work.

## Why I Built This

When I first learned about MLFQ in my OS class, I found the textbook explanations a bit abstract. Sure, I understood the theory, but I wanted to see it in action. Most online simulators were either too simple or didn't show the details I cared about. So I decided to build my own - one that would help me (and hopefully others) really understand what's happening under the hood.

## What You'll Find Here

### The Heart of It - Multiple Interfaces

I built three different ways to interact with the scheduler because everyone learns differently:

**Terminal Interface**: This is where I started. It's perfect when you want to step through the algorithm line by line and really understand what's happening. You can pause at any moment, examine the queue states, and see exactly why the scheduler made each decision.

**Web GUI**: This is probably my favorite part. It runs in your browser and communicates with the C++ backend to show everything in real-time - processes moving between queues, performance metrics updating live, and even visual indicators for when priority boosts happen.

**FLTK GUI**: A native desktop interface for those who prefer traditional applications. It's fast and responsive, great for running longer simulations.

### Real Algorithm Implementations

Here's something I'm particularly proud of - I didn't just implement basic round-robin for everything. The last queue can use three different algorithms:

- **Round Robin**: The classic approach, fair time-sharing
- **Shortest Job First**: Prioritizes shorter processes (great for system responsiveness)  
- **Priority Scheduling**: Uses aging to prevent starvation (my personal favorite)

I spent a lot of time making sure these work exactly like they do in real operating systems.

### Features That Actually Matter

**Boost Priority System**: This was tricky to get right. The system periodically moves all processes back to the highest priority queue to prevent starvation. You can see exactly when this happens and adjust the timing.

**Accurate Performance Metrics**: I double and triple-checked all the formulas. Wait time, turnaround time, response time, CPU utilization - everything matches what you'd find in OS textbooks.

**Algorithm Comparison Tool**: Want to see how SJF compares to Round Robin? This feature runs the same process set through different algorithms and shows you the results side by side.

## Getting Started (The Easy Way)

I tried to make this as simple as possible to run:

```bash
# Build everything (this handles dependencies automatically)
./build_web.sh

# Run the scheduler
cd build && ./mlfq_scheduler

# For the web interface, select option 5
# Then open http://localhost:8080 in your browser
```

That's it! The web interface is probably the best place to start - it's intuitive and shows everything visually.

## What I Learned Building This

### The Technical Challenges

**Quantum Implementation**: Getting the time quantum logic right was harder than I expected. Each queue needs to track how long a process has been running, handle quantum expiration correctly, and manage the transition between queues. I probably rewrote this part three times.

**Performance Metrics**: Calculating things like wait time and turnaround time sounds simple, but there are edge cases everywhere. What happens when a process gets boosted? How do you handle processes that haven't completed yet? I ended up creating a comprehensive test suite just for the metrics.

**Web Interface Synchronization**: Making sure the JavaScript simulation produces exactly the same results as the C++ backend was... interesting. I had to be very careful about timing and state management.

### The Design Decisions

**Why Three Interfaces?**: Different people learn differently. Some want to step through code, others prefer visual representations. I wanted to accommodate everyone.

**Why Focus on the Last Queue Algorithm?**: Most MLFQ implementations just use round-robin everywhere. But in real systems, you often want different behavior for long-running processes. This flexibility makes the scheduler much more interesting.

**Why Build a Web Interface?**: Honestly, because it's 2024 and web interfaces are just more accessible. No installation hassles, works on any device, and I could make it look really nice.

## How to Use This for Learning

### Start Simple
1. Run the web interface
2. Load the basic example process set
3. Watch a few complete simulations
4. Try different last-queue algorithms and see how they affect the results

### Dig Deeper  
1. Use the terminal interface to step through execution
2. Create your own process sets
3. Experiment with different boost intervals
4. Use the comparison tool to understand algorithm trade-offs

### Advanced Exploration
1. Modify the source code (it's well-commented!)
2. Try different queue configurations
3. Implement your own scheduling algorithm
4. Run performance tests with large process sets

## The Code Structure

I tried to keep things organized and readable:

```
src/           # Core C++ implementation
├── MLFQScheduler.cpp    # Main scheduling logic
├── Process.cpp          # Process management
├── Visualizer.cpp       # Terminal visualization
└── main.cpp            # User interface and menu system

web_gui/       # Web interface
├── index.html          # Main interface
├── script.js           # Frontend logic for the web interface
└── styles.css          # Visual styling

include/       # Header files with clear interfaces
docs/          # Documentation (you're reading it!)
```

## Performance and Limitations

I optimized this for learning, not production use. It can handle reasonable workloads (dozens of processes) without issues, but I prioritized code clarity over raw performance. The algorithms are correct and efficient, but there's definitely room for optimization if you need to handle thousands of processes.

## Things I'm Still Working On

- **Multi-core simulation**: Extending MLFQ to multiple CPUs
- **Real-time process injection**: Adding processes while simulation is running
- **More visualization options**: Different chart types, export capabilities
- **Mobile-friendly web interface**: Better responsive design

## Troubleshooting

**Web GUI won't load?** Check if port 8080 is available. You can change the port in the source if needed.

**Compilation issues?** Make sure you have a C++17 compatible compiler. The build script should handle most dependencies automatically.

**Results don't match expectations?** Double-check your process parameters and algorithm settings. The scheduler is deterministic - same inputs always produce same outputs.

## A Personal Note

This project taught me more about operating systems than any textbook ever could. There's something about implementing an algorithm from scratch that forces you to understand every detail. I hope it's as useful for your learning as it was for mine.

If you find bugs (and you probably will), or if you have ideas for improvements, I'd love to hear about them. This is very much a learning project, and I'm always looking to make it better.

## Acknowledgments

Huge thanks to my OS professor for encouraging this kind of deep dive into the material. The MLFQ algorithm itself comes from the classic operating systems literature - I just tried to implement it faithfully and make it accessible.

The web interface design was inspired by modern development tools and educational simulators, but all the code is my own work.

---

*Happy scheduling! I hope this helps you understand MLFQ as much as building it helped me.*

**- A fellow OS student who got a bit carried away with a class project**

```
END-SEM-PROJECT/
├── include/
│   ├── Process.h           # Process Control Block definition
│   ├── Queue.h             # Process queue management
│   ├── MLFQScheduler.h     # Core MLFQ scheduling algorithm
│   ├── Visualizer.h        # Terminal-based visualization
│   ├── FLTKVisualizer.h    # GUI visualization
│   ├── SchedulerConfig.h   # Configuration management
│   └── TerminalUI.h        # Terminal UI utilities
├── src/
│   ├── Process.cpp         # Process implementation
│   ├── Queue.cpp           # Queue operations
│   ├── MLFQScheduler.cpp   # Scheduler logic
│   ├── Visualizer.cpp      # Visualization implementation
│   ├── FLTKVisualizer.cpp  # GUI visualization implementation
│   └── main.cpp            # Main program with UI
├── build/                  # Build directory (generated)
├── docs/                   # Documentation
├── tests/                  # Unit tests
├── CMakeLists.txt          # CMake build configuration
├── README.md               # This file
└── plan.txt                # Original project plan

```

## MLFQ Algorithm Rules

The implementation follows these core MLFQ rules:

1. **Rule 1**: If Priority(A) > Priority(B), then A runs before B
2. **Rule 2**: If Priority(A) == Priority(B), A and B run in Round-Robin
3. **Rule 3**: When a process uses its time quantum, it moves to a lower priority queue
4. **Rule 4**: After period S (boost interval), all processes move to the highest priority queue

## Building the Project

### Prerequisites
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10 or higher
- Make or Ninja build system
- FLTK libraries (optional, for GUI support)

### Build Instructions

```bash
# Navigate to project directory
cd END-SEM-PROJECT

# Create build directory
mkdir -p build
cd build

# Configure with CMake (with FLTK support if available)
cmake ..

# Build the project
make

# Run the executable
./mlfq_scheduler
```

### Alternative: Direct Compilation

```bash
# Compile all source files
g++ -std=c++17 -I./include \
    src/Process.cpp \
    src/Queue.cpp \
    src/MLFQScheduler.cpp \
    src/Visualizer.cpp \
    src/main.cpp \
    -o mlfq_scheduler

# Run with FLTK support (if available)
g++ -std=c++17 -I./include -DFLTK_AVAILABLE \
    src/Process.cpp \
    src/Queue.cpp \
    src/MLFQScheduler.cpp \
    src/Visualizer.cpp \
    src/FLTKVisualizer.cpp \
    src/main.cpp \
    -lfltk -o mlfq_scheduler
```

## Usage

### Running the Program

When you run the program, you'll see an expanded menu with options:

1. **Run with example process set** - Execute with predefined processes
2. **Create custom processes** - Define your own process set
3. **Generate random processes** - Use configurable random generation
4. **Run with GUI visualization** - Launch FLTK-based GUI (if FLTK available)
5. **Run with algorithm-switching scheduler** - Use alternative algorithms for last queue
6. **Configure scheduler parameters** - Interactive parameter configuration
7. **Load preset configuration** - Use predefined configurations
8. **Load example process set with default values** - Select example set to use globally
9. **Exit** - Quit the program

### Interactive Mode

- Press **Enter** to step through one time unit
- Press **'a'** to switch to automatic mode
- Press **'q'** to quit current simulation

### Understanding the Display

```
┌─────────────────────────────────────────────────────────┐
│ CURRENT EXECUTION (Time:   10)                          │
├─────────────────────────────────────────────────────────┤
│ Running: P01 [Queue:0 Arr:  0 Burst: 24 Rem: 14 Wait:  0│
│ Progress: [==========                    ] 10/24        │
└─────────────────────────────────────────────────────────┘

╔═══════════════════════════════════════════════════════════╗
║              MULTILEVEL FEEDBACK QUEUES                   ║
╠═══════════════════════════════════════════════════════════╣
║ Queue 0 (Quantum:  4ms) [1 processes]
║   P02
║ Queue 1 (Quantum:  8ms) [0 processes]
║   (empty)
║ Queue 2 (Quantum: 16ms) [0 processes]
║   (empty)
╚═══════════════════════════════════════════════════════════╝
```

## Code Architecture

### Class Hierarchy

1. **Process**: Represents a single process with all its attributes
   - Tracks state, timing, and metrics
   - Handles execution and metric calculation

2. **ProcessQueue**: Manages a single priority level queue
   - FIFO queue operations
   - Round-robin support

3. **MLFQScheduler**: Core scheduling algorithm
   - Manages multiple queues
   - Implements MLFQ rules
   - Tracks statistics
   - Supports algorithm switching for last queue

4. **Visualizer**: Terminal-based visualization
   - Displays queue states
   - Shows execution progress
   - Presents statistics

5. **FLTKVisualizer**: GUI visualization (optional)
   - FLTK-based interface
   - Interactive visualization
   - Real-time process monitoring

6. **SchedulerConfig**: Configuration management
   - Parameter storage and validation
   - Preset configurations
   - Runtime configuration

7. **TerminalUI**: Terminal utilities
   - Formatting and styling
   - Screen clearing and headers

### Key Design Patterns

- **Modular Design**: Each class has a single responsibility
- **Separation of Concerns**: Logic separated from visualization
- **Smart Pointers**: Using `std::shared_ptr` for safe memory management
- **STL Containers**: Leveraging standard library for efficiency
- **Conditional Compilation**: Optional FLTK GUI support

## Scheduling Metrics

The scheduler calculates:

- **Wait Time**: Time spent in ready queue
- **Turnaround Time**: Total time from arrival to completion
- **Response Time**: Time from arrival to first execution
- **CPU Utilization**: Percentage of time CPU is busy

## Configuration Options

The project supports runtime configuration with the following parameters:

- **Number of Queues** (2-5 queues)
- **Base Time Quantum** (2-10 ms)
- **Quantum Growth Factor** (1.0-5.0x)
- **Priority Boost Interval** (20-500 ms)
- **Animation Delay** (50-2000 ms)
- **Process Generation Parameters** (number, arrival time, burst time range)

## Example Process Sets

### Set 1 (Standard)
| Process | Arrival | Burst |
|---------|---------|-------|
| P1      | 0       | 20    |
| P2      | 5       | 12    |
| P3      | 10      | 8     |
| P4      | 15      | 16    |
| P5      | 20      | 5     |

### Set 2 (CPU-Intensive)
| Process | Arrival | Burst |
|---------|---------|-------|
| P1      | 0       | 30    |
| P2      | 5       | 25    |
| P3      | 10      | 20    |

## Algorithm Switching

The scheduler supports algorithm switching on the last queue:

- **Round Robin (RR)** - Default round-robin scheduling
- **Shortest Job First (SJF)** - Processes with shortest remaining time execute first
- **Priority Scheduling** - Processes with highest priority execute first

## References

- Operating System Concepts (Silberschatz, Galvin, Gagne)
- MLFQ Scheduler: http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-mlfq.pdf
- Modern Operating Systems (Andrew S. Tanenbaum)
- FLTK Documentation: https://www.fltk.org/doc-1.3/
