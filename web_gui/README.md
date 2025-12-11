# MLFQ Scheduler - Web GUI

A modern, professional web interface for the Multilevel Feedback Queue (MLFQ) Scheduler with real-time animations and interactive controls.

## Features

### 🎨 Professional Design
- Modern gradient backgrounds and smooth animations
- Responsive design that works on all screen sizes
- Clean, intuitive user interface with professional color scheme
- Visual hierarchy with consistent typography and spacing

### 📊 Real-time Visualization
- Live queue status with color-coded priority levels
- Individual process ID display in queues (P1, P2, P3 instead of "3 processes")
- Animated process movements between queues
- Running process highlighting with pulse animation
- Dynamic process table updates with real-time state changes

### 🎮 Interactive Controls
- Start/Pause/Reset simulation controls
- Step-by-step execution mode for detailed analysis
- Adjustable simulation speed (1x to 10x)
- Add custom processes via modal dialog
- Multiple preset process sets with different characteristics

### 📈 Performance Metrics Dashboard
- Real-time performance metrics calculation and display
- Average wait time tracking
- Average turnaround time monitoring
- CPU utilization percentage
- Throughput measurement (processes per time unit)
- Priority boost information with countdown timer

### ⚡ Advanced Features
- Complete JavaScript MLFQ implementation mirroring C++ backend
- Process state management (NEW, READY, RUNNING, TERMINATED)
- Queue-specific time quantum display
- Real-time priority boost timer with countdown
- Hover effects and smooth CSS transitions
- Modal dialogs for process creation and configuration
- Automatic metrics updates during simulation

## Recent Improvements

### Enhanced Queue Display
- **Before**: Queues showed only process count (e.g., "3 processes")
- **After**: Queues show individual process IDs (e.g., "P1 P3 P5")
- Allows tracking of specific processes as they move between queues

### Multiple Preset Process Sets
- **Standard Set**: Mixed workload with 5 processes
- **CPU-Intensive**: Long-running processes (3 processes) 
- **I/O-Intensive**: Short burst processes (5 processes)
- **Mixed Workload**: Combination of short and long processes (7 processes)

### Real-time Priority Boost Display
- Live countdown timer showing time until next priority boost
- Updates dynamically during simulation
- Shows last boost time and next boost prediction

## How to Use

1. **Build and Run**:
   ```bash
   ./build_web.sh
   cd build
   ./mlfq_scheduler
   ```

2. **Select Web GUI**: Choose option 5 from the main menu

3. **Open Browser**: Navigate to `http://localhost:8080`

4. **Load Processes**: 
   - Click "📋 Preset Processes" to choose from 4 different process sets
   - Or click "➕ Add Process" to create custom processes
   - Or click "🎲 Random Processes" to generate random workloads

5. **Configure Simulation**:
   - Click "⚙️ Config" to adjust scheduler parameters
   - Modify queue algorithms, boost intervals, and time quantums
   - Changes take effect immediately

6. **Control Simulation**:
   - Use ▶️ Start/⏸️ Pause/🔄 Reset buttons
   - Adjust speed with the slider (1x to 10x)
   - Use ⏭️ Step button for detailed step-by-step analysis

## Technical Implementation

### Frontend Architecture
- **HTML5**: Semantic structure with accessibility features and ARIA labels
- **CSS3**: Modern styling with flexbox/grid layouts, CSS animations, and responsive design
- **JavaScript ES6+**: Class-based architecture with modular design

### MLFQ Algorithm Implementation
The JavaScript implementation includes:
- Complete MLFQ scheduling logic identical to C++ backend
- Multi-queue management with priority levels
- Time quantum progression (1, 2, 4, 8, ...)
- Priority boost mechanism (aging prevention)
- Support for different last-queue algorithms (RR, SJF, Priority)
- Accurate performance metric calculations

### Backend Integration
- **C++ Web Server**: Lightweight HTTP server using standard sockets
- **Static File Serving**: Serves HTML, CSS, and JavaScript files
- **Cross-platform**: Works on Linux, macOS, and Windows
- **No External Dependencies**: Built-in web server, no Apache/Nginx needed

### Key JavaScript Classes

```javascript
class MLFQWebInterface {
    constructor() {
        this.queues = [];           // Priority queues
        this.processes = [];        // All processes
        this.currentTime = 0;       // System clock
        this.boostTimer = 0;        // Priority boost timer
        this.config = {...};        // Scheduler configuration
    }
    
    step() {                        // Execute one time unit
        this.handleArrivals();
        this.executeCurrentProcess();
        this.updateMetrics();
        this.checkPriorityBoost();
        this.updateDisplay();
    }
}

class Process {
    constructor(id, arrival, burst) {
        this.id = id;
        this.arrivalTime = arrival;
        this.burstTime = burst;
        this.remainingTime = burst;
        this.state = 'NEW';
        // ... other properties
    }
}
```

### Visualization Features
1. **Queue Animation**: Smooth transitions when processes move between queues
2. **Process Highlighting**: Running process pulses with animation
3. **Real-time Updates**: All displays update synchronously
4. **Responsive Design**: Adapts to different screen sizes
5. **Professional Styling**: Modern gradients and color schemes

## Browser Compatibility

- **Chrome 60+**: Full support with hardware acceleration
- **Firefox 55+**: Full support with smooth animations
- **Safari 12+**: Full support on macOS and iOS
- **Edge 79+**: Full support with Chromium engine

## File Structure

```
web_gui/
├── index.html          # Main HTML structure (~350 lines)
├── styles.css          # Professional styling and animations (~250 lines)
├── script.js           # Interactive functionality (~400 lines)
└── README.md          # This documentation
```

## Performance Optimizations

- **Lightweight**: Total size < 100KB for fast loading
- **GPU Acceleration**: CSS animations use transform and opacity
- **Efficient DOM Updates**: Minimal manipulation for smooth performance
- **Responsive Rendering**: 60fps animations on modern browsers
- **Memory Management**: Efficient process and queue management

## Customization Options

### Visual Themes
Modify CSS variables for easy theme changes:
```css
:root {
    --primary-color: #667eea;
    --secondary-color: #764ba2;
    --success-color: #10b981;
    --warning-color: #f59e0b;
    --danger-color: #ef4444;
}
```

### Animation Settings
Adjust animation durations and easing:
```css
.process-move {
    transition: all 0.5s cubic-bezier(0.4, 0, 0.2, 1);
}
```

### Layout Modifications
The responsive grid system is easily adaptable:
```css
.main-container {
    display: grid;
    grid-template-columns: 1fr 300px;
    gap: 20px;
}
```

## Educational Value

The web interface serves as an excellent educational tool by:

1. **Visual Learning**: Students can see exactly how MLFQ works
2. **Interactive Experimentation**: Change parameters and see immediate effects
3. **Step-by-Step Analysis**: Understand each scheduling decision
4. **Algorithm Comparison**: Compare different approaches side-by-side
5. **Real Metrics**: Learn how performance is measured in operating systems

## Future Enhancements

### Planned Features
- **Mobile Optimization**: Enhanced touch interface for tablets
- **Export Capabilities**: Save simulation results as PDF or CSV
- **Advanced Animations**: Slow-motion replay and timeline scrubbing
- **Process Creation Wizard**: Guided process set creation
- **Algorithm Visualization**: Show decision trees and algorithm steps

### Technical Improvements
- **WebAssembly Integration**: Port C++ algorithm for perfect consistency
- **Real-time Collaboration**: Multiple users viewing same simulation
- **Performance Profiling**: Detailed timing and memory usage analysis
- **Accessibility Enhancements**: Screen reader support and keyboard navigation

This web GUI provides a professional, modern alternative to desktop interfaces while maintaining all the functionality and educational value of the MLFQ scheduler with enhanced visual appeal and interactive capabilities.
