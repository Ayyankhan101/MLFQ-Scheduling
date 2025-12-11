# Project Structure

MLFQ-Scheduling/
│
├── include/                      # Header files
│   ├── Process.h                 # Process Control Block definition
│   ├── Queue.h                   # Process queue management
│   ├── MLFQScheduler.h           # MLFQ scheduling algorithm
│   ├── Visualizer.h              # Terminal visualization
│   ├── FLTKVisualizer.h          # GUI visualization (optional)
│   ├── SchedulerConfig.h         # Configuration management
│   ├── TerminalUI.h              # Terminal UI utilities
│   └── WebServer.h               # Web server implementation
│
├── src/                          # Source files
│   ├── Process.cpp               # Process implementation (~100 lines)
│   ├── Queue.cpp                 # Queue operations (~50 lines)
│   ├── MLFQScheduler.cpp         # Scheduler logic (~500 lines)
│   ├── Visualizer.cpp            # Terminal visualization (~300 lines)
│   ├── FLTKVisualizer.cpp        # GUI visualization (~400 lines)
│   ├── WebServer.cpp             # Web server implementation (~350 lines)
│   └── main.cpp                  # Main program with UI (~1200 lines)
│
├── web_gui/                      # Web interface
│   ├── index.html                # Main HTML structure (~350 lines)
│   ├── styles.css                # Professional styling (~250 lines)
│   ├── script.js                 # Interactive functionality (~400 lines)
│   └── README.md                 # Web GUI documentation
│
├── tests/                        # Unit tests
│   └── test_scheduler.cpp        # Test suite (~100 lines)
│
├── docs/                         # Documentation
│   ├── TECHNICAL.md              # Technical documentation
│   ├── QUICKSTART.md             # Quick start guide
│   ├── STRUCTURE.md              # This file - Project structure
│   ├── CONFIGURATION_GUIDE.md    # Configuration options guide
│   ├── PROJECT_SUMMARY.md        # Project completion summary
│   └── README.md                 # Main documentation
│
├── build/                        # Build artifacts (generated)
│   ├── mlfq_scheduler            # Main executable
│   ├── test_scheduler            # Test executable
│   └── CMakeFiles/               # CMake build files
│
├── logs/                         # Log files (generated)
│   └── openai/                   # OpenAI API logs
│
├── CMakeLists.txt                # CMake configuration
├── build.sh                      # Basic build script
├── build_web.sh                  # Web-enabled build script
├── LICENSE                       # MIT License
├── README.md                     # Main documentation
├── .gitignore                    # Git ignore rules
└── test_web_improvements.md      # Web improvements test log

## Key Files Description

### Header Files (include/)

**Process.h** (~80 lines)
- Defines ProcessState enum (NEW, READY, RUNNING, TERMINATED)
- Process class with PCB attributes (PID, arrival time, burst time, etc.)
- Methods for execution, metrics calculation, state management

**Queue.h** (~40 lines)
- ProcessQueue class for FIFO queue operations
- Round-robin support with time quantum tracking
- Queue statistics and management

**MLFQScheduler.h** (~120 lines)
- Core MLFQ scheduler class definition
- Multiple queue management with different algorithms
- Statistics tracking and performance metrics
- Last queue algorithm selection (RR, SJF, Priority)
- Configuration-based constructor support

**Visualizer.h** (~35 lines)
- Terminal display functions and formatting
- Progress bars, statistics display, Gantt charts
- Queue visualization in text mode

**FLTKVisualizer.h** (~80 lines)
- GUI visualization implementation using FLTK
- Cross-platform desktop interface
- Real-time process monitoring and controls

**SchedulerConfig.h** (~400 lines)
- Comprehensive configuration management
- Scheduler parameter definitions and validation
- Multiple configuration presets
- Quantum calculation methods

**TerminalUI.h** (~150 lines)
- Terminal UI utilities and styling
- Color formatting, clear screen, headers
- Cross-platform terminal compatibility

**WebServer.h** (~25 lines)
- HTTP server interface for web GUI
- Static file serving capabilities
- Cross-platform socket implementation

### Source Files (src/)

**Process.cpp** (~100 lines)
- Process execution logic and state transitions
- Performance metric calculations (wait time, turnaround time)
- Process control block management

**Queue.cpp** (~50 lines)
- Queue enqueue/dequeue operations
- Process management within queues
- Time quantum and round-robin support

**MLFQScheduler.cpp** (~500 lines)
- Complete MLFQ algorithm implementation
- Multi-queue management with priority levels
- Time quantum progression (1, 2, 4, 8, ...)
- Priority boost mechanism (aging prevention)
- Statistics calculation and performance metrics
- Support for different last-queue algorithms
- Configuration-based initialization

**Visualizer.cpp** (~300 lines)
- Terminal-based visualization and formatting
- Queue state display with process information
- Statistics panel with real-time updates
- Gantt chart generation and display
- Process table with comprehensive information

**FLTKVisualizer.cpp** (~400 lines)
- Native GUI implementation using FLTK library
- Interactive visualization with real-time updates
- Desktop application interface
- Configuration controls and process management

**WebServer.cpp** (~350 lines)
- Built-in HTTP server implementation
- Static file serving for web GUI
- Cross-platform socket programming
- MIME type handling for web assets

**main.cpp** (~1200 lines)
- Comprehensive menu system with 11+ options
- Interactive and automatic execution modes
- Process set management (example, custom, random)
- Algorithm comparison tools
- Configuration management interface
- CSV export functionality
- Web GUI integration

### Web Interface (web_gui/)

**index.html** (~350 lines)
- Modern, responsive HTML5 structure
- Professional layout with flexbox/grid
- Interactive controls and modals
- Accessibility features and semantic markup

**styles.css** (~250 lines)
- Professional styling with CSS3
- Smooth animations and transitions
- Responsive design for all screen sizes
- Modern color scheme and typography

**script.js** (~400 lines)
- Complete JavaScript MLFQ implementation
- Real-time visualization and animations
- Interactive process management
- Performance metrics calculation
- Multiple preset process sets
- Configuration management

## Module Dependencies

```
main.cpp
  ├─ TerminalUI (styling and formatting)
  ├─ SchedulerConfig (configuration management)
  ├─ MLFQScheduler (core algorithm)
  │   ├─ Process (process control blocks)
  │   ├─ Queue (queue management)
  │   └─ SchedulerConfig (parameters)
  ├─ Visualizer (terminal display)
  │   └─ MLFQScheduler
  ├─ WebServer (web interface)
  │   └─ Static file serving
  └─ FLTKVisualizer (optional GUI)
       └─ MLFQScheduler
```

## Build System

### CMakeLists.txt Features
- Automatic FLTK detection (optional dependency)
- Cross-platform compatibility (Linux, macOS, Windows)
- C++17 standard requirement
- Separate executables for main program and tests
- Conditional compilation for GUI features

### Build Scripts
- **build.sh**: Basic build with CMake
- **build_web.sh**: Enhanced build with web GUI support and instructions

## Build Outputs

- `mlfq_scheduler` - Main executable (with or without FLTK GUI support)
- `test_scheduler` - Test executable for unit testing
- Web assets served directly from `web_gui/` directory

## Recent Improvements

### Web Interface Enhancements
- Individual process ID display in queues (P1, P2, P3 instead of "3 processes")
- Multiple preset process sets (Standard, CPU-Intensive, I/O-Intensive, Mixed)
- Real-time priority boost timer with countdown display
- Enhanced visual feedback and animations

### Code Organization
- Modular architecture with clear separation of concerns
- Configuration-driven design for easy parameter adjustment
- Comprehensive error handling and validation
- Extensive documentation and comments

## Testing Infrastructure

### Unit Tests (tests/)
- Process creation and state management
- Queue operations and round-robin scheduling
- Scheduler logic and algorithm correctness
- Performance metric calculations
- Edge cases and error conditions

### Integration Testing
- Web interface consistency with C++ backend
- Cross-platform compatibility verification
- Performance testing with large process sets
- Algorithm comparison validation

