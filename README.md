# MLFQ Scheduler - Multilevel Feedback Queue Implementation

A comprehensive implementation of the Multilevel Feedback Queue (MLFQ) scheduling algorithm with multiple visualization interfaces and educational features.

![MLFQ Scheduler Demo](https://img.shields.io/badge/Status-Complete-brightgreen)
![C++](https://img.shields.io/badge/C++-17-blue)
![Web](https://img.shields.io/badge/Web-HTML5%2FJS-orange)
![License](https://img.shields.io/badge/License-MIT-green)

## 🚀 Quick Start

```bash
# Clone the repository
git clone https://github.com/Ayyankhan101/MLFQ-Scheduling.git
cd MLFQ-Scheduling

# Build and run
./build_web.sh
cd build && ./mlfq_scheduler

# Choose option 5 for Web GUI, then open http://localhost:8080
```

## ✨ Features

- **🎯 Multiple Interfaces**: Terminal, Web GUI, and FLTK GUI
- **⚡ Real-time Visualization**: Watch processes move between queues
- **📊 Performance Metrics**: Accurate wait time, turnaround time, CPU utilization, and throughput
- **🔄 Algorithm Comparison**: Compare Round Robin, SJF, and Priority Scheduling
- **🎛️ Configurable Parameters**: Adjust quantum times, boost intervals, queue counts
- **📈 Educational Tools**: Step-by-step execution and detailed explanations
- **🎯 Consistent Configuration**: All parameters are now applied consistently across all execution modes and algorithm comparisons

## 🖥️ Interface Options

### Web Interface (Recommended)
Modern, interactive visualization that runs in any browser with:
- Real-time queue animations
- Professional design with smooth transitions
- Interactive process management
- Performance metrics dashboard
- Multiple preset process sets
- Step-by-step simulation control

### Terminal Interface  
Command-line interface perfect for understanding the algorithm:
- Step-by-step execution mode
- Detailed scheduler decision output
- Comprehensive performance statistics
- Gantt chart visualization
- Algorithm comparison tools

### FLTK GUI (Optional)
Native desktop application with:
- Cross-platform compatibility
- Fast rendering for large simulations
- Traditional desktop interface
- Real-time process monitoring

## 🏗️ Architecture

```
MLFQ-Scheduling/
├── src/                    # Core C++ implementation
│   ├── MLFQScheduler.cpp   # Core MLFQ algorithm
│   ├── WebServer.cpp       # Built-in HTTP server
│   ├── main.cpp            # Main program with menu system
│   ├── Process.cpp         # Process control block
│   ├── Queue.cpp           # Queue management
│   ├── Visualizer.cpp      # Terminal visualization
│   └── FLTKVisualizer.cpp  # GUI visualization (optional)
├── include/                # Header files
│   ├── MLFQScheduler.h     # MLFQ scheduler definition
│   ├── SchedulerConfig.h   # Configuration management
│   ├── Process.h           # Process definition
│   └── ...                 # Other headers
├── web_gui/                # Web interface
│   ├── index.html          # Main web interface
│   ├── script.js           # JavaScript frontend for the web interface
│   └── styles.css          # Professional styling
├── tests/                  # Unit tests (currently empty after removal)
├── docs/                   # Documentation
│   ├── QUICKSTART.md       # 5-minute setup guide
│   ├── TECHNICAL.md        # Implementation details
│   ├── PROJECT_SUMMARY.md  # Learning journey
│   └── CONFIGURATION_GUIDE.md # Parameter tuning
└── build/                  # Compiled binaries
```

## 🌐 Web GUI Architecture

The project features a unique **C++-to-Web integration** that serves a complete JavaScript-based MLFQ scheduler:

### Server-Side (C++):
- **WebServer.cpp**: Built-in HTTP server that serves static files
- **Built-in web server**: No external dependencies needed
- **Cross-platform**: Works on Linux, macOS, and Windows

### Client-Side (JavaScript):
- **Complete MLFQ implementation**: JavaScript version mirrors the C++ algorithm
- **Real-time visualization**: Processes move between queues in real-time
- **Interactive controls**: Start, pause, reset, configure, and step through execution
- **Performance metrics**: Live calculation and display of scheduling metrics

### Integration Pattern:
1. C++ application starts a web server on port 8080
2. Web server serves HTML, CSS, and JavaScript files to browser
3. JavaScript runs independently in browser, implementing the full MLFQ algorithm
4. No continuous communication needed between C++ and JavaScript
5. Both implementations produce identical results for consistency

## 🎓 Educational Value

This project was built to help students understand MLFQ scheduling through:

- **Visual Learning**: See exactly how processes move between priority queues
- **Interactive Experimentation**: Change parameters and see immediate effects  
- **Algorithm Comparison**: Understand trade-offs between different approaches
- **Real Metrics**: Learn how performance is measured in operating systems

## 🔧 Technical Details

### Core Algorithm Features
- ✅ Multiple priority queues with exponential quantum progression
- ✅ Round-robin scheduling within each queue
- ✅ Priority boost (aging) to prevent starvation
- ✅ Configurable last-queue algorithms (RR/SJF/Priority)
- ✅ Accurate performance metric calculations

### Supported Platforms
- **Linux** (primary development platform)
- **macOS** (tested and working)
- **Windows** (should work with minor modifications)
- **Web browsers** (Chrome, Firefox, Safari, Edge)

## 📚 Documentation

- **[Quick Start Guide](docs/QUICKSTART.md)** - Get running in 5 minutes
- **[Project Summary](docs/PROJECT_SUMMARY.md)** - Detailed overview and learning journey
- **[Technical Documentation](docs/TECHNICAL.md)** - Implementation details
- **[Configuration Guide](docs/CONFIGURATION_GUIDE.md)** - Parameter tuning

## 🧪 Testing

Unit tests were previously available but have been removed in recent updates:

Tests previously covered:
- Process creation and management
- Queue operations
- Scheduler logic
- Performance metric calculations
- Edge cases and error conditions

## 🎯 Usage Examples

### Basic Simulation
```bash
./mlfq_scheduler
# Choose option 1 for example processes
# Select interactive mode to step through execution
```

### Algorithm Comparison
```bash
./mlfq_scheduler  
# Choose option 10 (with FLTK) or 8 (without FLTK)
# Compare performance of different last-queue algorithms
```

### Web Interface
```bash
./mlfq_scheduler
# Choose option 5
# Open http://localhost:8080
# Use Config button to experiment with different settings
```

## 🤝 Contributing

This is an educational project, but improvements are welcome! Areas for enhancement:

- Multi-core MLFQ simulation
- Additional visualization options
- Mobile-friendly web interface
- More scheduling algorithm variants

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📞 Contact

If you have questions about the implementation or find this useful for learning, feel free to reach out!