# Project Structure

END-SEM-PROJECT/
│
├── include/                      # Header files
│   ├── Process.h                 # Process Control Block definition
│   ├── Queue.h                   # Process queue management
│   ├── MLFQScheduler.h           # MLFQ scheduling algorithm
│   ├── Visualizer.h              # Terminal visualization
│   ├── FLTKVisualizer.h          # GUI visualization
│   ├── SchedulerConfig.h         # Configuration management
│   └── TerminalUI.h              # Terminal UI utilities
│
├── src/                          # Source files
│   ├── Process.cpp               # Process implementation
│   ├── Queue.cpp                 # Queue operations
│   ├── MLFQScheduler.cpp         # Scheduler logic
│   ├── Visualizer.cpp            # Terminal visualization implementation
│   ├── FLTKVisualizer.cpp        # GUI visualization implementation
│   └── main.cpp                  # Main program with UI
│
├── tests/                        # Unit tests
│   └── test_scheduler.cpp        # Test suite
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
│
├── CMakeLists.txt                # CMake configuration
├── Makefile                      # Make build configuration (if exists)
├── build.sh                      # Build script
├── README.md                     # Main documentation
└── plan.txt                      # Original project plan

## Key Files Description

### Header Files (include/)

**Process.h**
- Defines ProcessState enum
- Process class with PCB attributes
- Methods for execution, metrics calculation

**Queue.h**
- ProcessQueue class
- FIFO queue operations
- Round-robin support

**MLFQScheduler.h**
- Core MLFQ scheduler class
- Multiple queue management
- Statistics tracking
- Last queue algorithm selection (RR, SJF, Priority)

**Visualizer.h**
- Terminal display functions
- Progress bars and formatting
- Statistics display

**FLTKVisualizer.h**
- GUI visualization implementation
- FLTK-based user interface
- Cross-platform compatibility

**SchedulerConfig.h**
- Configuration management class
- Scheduler parameter definitions
- Configuration presets

**TerminalUI.h**
- Terminal UI utilities
- Formatting and styling
- Clear screen, headers, etc.

### Source Files (src/)

**Process.cpp** (~50 lines)
- Process execution logic
- Metric calculations
- State management

**Queue.cpp** (~40 lines)
- Queue enqueue/dequeue
- Process management in queues

**MLFQScheduler.cpp** (~200 lines)
- MLFQ algorithm implementation
- Time quantum management
- Priority boost mechanism
- Statistics calculation
- Last queue algorithm switching

**Visualizer.cpp** (~150 lines)
- Display formatting
- Queue visualization
- Statistics panel
- Gantt chart generation
- Process table display

**FLTKVisualizer.cpp** (~250 lines)
- GUI implementation using FLTK
- Interactive visualization
- Real-time process monitoring
- Configuration controls

**main.cpp** (~700 lines)
- User interface with multiple modes
- Menu system (GUI, algorithm switching, configuration)
- Interactive/automatic modes
- Process set configurations
- CSV results export
- Configuration management

## Module Dependencies

```
main.cpp
  ├─ TerminalUI
  ├─ SchedulerConfig
  ├─ Visualizer
  │   └─ MLFQScheduler
  │        ├─ Process
  │        └─ Queue
  │             └─ Process
  └─ FLTKVisualizer (optional)
       └─ MLFQScheduler
```

## Build Outputs

- `mlfq_scheduler` - Main executable (with or without GUI support)
- `test_scheduler` - Test executable

