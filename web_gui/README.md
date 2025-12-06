# MLFQ Scheduler - Web GUI

A modern, professional web interface for the Multilevel Feedback Queue (MLFQ) Scheduler with real-time animations and interactive controls.

## Features

### 🎨 Professional Design
- Modern gradient backgrounds and smooth animations
- Responsive design that works on all screen sizes
- Clean, intuitive user interface
- Professional color scheme with visual hierarchy

### 📊 Real-time Visualization
- Live queue status with color-coded priority levels
- Animated process movements between queues
- Running process highlighting with pulse animation
- Dynamic process table updates

### 🎮 Interactive Controls
- Start/Pause/Reset simulation controls
- Step-by-step execution mode
- Adjustable simulation speed (1x to 10x)
- Add custom processes via modal dialog
- Load example process sets

### 📈 Performance Metrics
- Real-time performance dashboard
- Average wait time calculation
- Average turnaround time tracking
- CPU utilization percentage
- Throughput measurement

### ⚡ Advanced Features
- Process state management (NEW, READY, RUNNING, TERMINATED)
- Queue-specific time quantum display
- Hover effects and smooth transitions
- Modal dialogs for process creation
- Automatic metrics updates

## How to Use

1. **Build and Run**:
   ```bash
   ./build_web.sh
   cd build
   ./mlfq_scheduler
   ```

2. **Select Web GUI**: Choose option 5 from the main menu

3. **Open Browser**: Navigate to `http://localhost:8080`

4. **Add Processes**: 
   - Click "Add Process" to create custom processes
   - Or click "Load Example" for predefined process sets

5. **Control Simulation**:
   - Use Start/Pause/Reset buttons
   - Adjust speed with the slider
   - Use Step button for detailed analysis

## Technical Implementation

### Frontend
- **HTML5**: Semantic structure with accessibility features
- **CSS3**: Modern styling with flexbox/grid layouts, animations, and responsive design
- **JavaScript**: ES6+ with class-based architecture for clean code organization

### Backend Integration
- **C++ Web Server**: Lightweight HTTP server using standard sockets
- **Real-time Communication**: HTTP-based communication between frontend and C++ backend
- **Cross-platform**: Works on Linux, macOS, and Windows

### Key Components

1. **MLFQWebInterface Class**: Main JavaScript controller
2. **WebServer Class**: C++ HTTP server implementation
3. **Responsive CSS**: Mobile-first design approach
4. **Animation System**: CSS keyframes for smooth transitions

## Browser Compatibility

- Chrome 60+
- Firefox 55+
- Safari 12+
- Edge 79+

## File Structure

```
web_gui/
├── index.html          # Main HTML structure
├── styles.css          # Professional styling and animations
├── script.js           # Interactive functionality
└── README.md          # This documentation
```

## Customization

The web interface is highly customizable:

- **Colors**: Modify CSS variables for theme changes
- **Animations**: Adjust keyframe animations in styles.css
- **Layout**: Responsive grid system easily adaptable
- **Features**: Modular JavaScript architecture for easy extension

## Performance

- Lightweight: < 100KB total size
- Fast rendering: CSS animations use GPU acceleration
- Efficient updates: Minimal DOM manipulation
- Responsive: 60fps animations on modern browsers

This web GUI provides a professional alternative to the FLTK desktop interface while maintaining all the functionality of your MLFQ scheduler with enhanced visual appeal and modern web standards.
