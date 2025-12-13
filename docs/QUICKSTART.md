# Quick Start Guide - Get Running in 5 Minutes

## TL;DR - Just Want to See It Work?

```bash
git clone https://github.com/Ayyankhan101/MLFQ-Scheduling.git
cd MLFQ-Scheduling
./build_web.sh
cd build && ./mlfq_scheduler
# Choose option 5, then open http://localhost:8080
```

Done! You should see the web interface with a beautiful MLFQ visualization.

## First Time Setup

### What You Need
- Any modern Linux system (developed on Ubuntu)
- GCC with C++17 support (most recent versions work fine)
- A web browser (Chrome, Firefox, Safari - anything modern)

### Building the Project

I made this as painless as possible:

```bash
# This script handles everything - dependencies, compilation, the works
./build_web.sh
```

If you see "Build successful!" at the end, you're good to go. If not, check that you have a C++ compiler installed.

### Your First Simulation

1. **Start the program:**
   ```bash
   cd build
   ./mlfq_scheduler
   ```

2. **Choose the web interface (option 5)**
   - This is honestly the best way to start
   - Much more intuitive than the terminal version

3. **Open your browser to http://localhost:8080**
   - You should see a clean, modern interface
   - Don't worry if it takes a second to load

4. **Load some processes:**
   - Click the "📋 Preset Processes" button
   - Choose from 4 different preset sets:
     - **Standard Set**: Mixed workload (5 processes)
     - **CPU-Intensive**: Long-running processes (3 processes)
     - **I/O-Intensive**: Short processes (5 processes)
     - **Mixed Workload**: Combination of short and long (7 processes)

5. **Start the simulation:**
   - Click "▶️ Start Simulation"
   - Watch the magic happen!

## What You're Seeing

### The Queue Visualization
Those colored boxes represent your priority queues:
- **Queue 0**: Highest priority
- **Queue 1**: Medium priority  
- **Queue 2**: Lower priority
- **Queue 3**: Lowest priority

(The quantum values are configurable in the application)

Processes move between these queues based on how much CPU time they use. You'll see individual process IDs (P1, P2, etc.) displayed in each queue.

### The Process Table
This shows every process and its current state:
- **ID**: Process identifier (P1, P2, etc.)
- **Arrival**: When the process first showed up
- **Burst**: How much CPU time it needs total
- **Remaining**: How much CPU time is left
- **State**: What's happening right now (NEW, READY, RUNNING, TERMINATED)
- **Completion Time**: When it finished (if it has)
- **Wait Time**: How long it spent waiting
- **Turnaround**: Total time from arrival to completion

### Performance Metrics
The dashboard on the right shows:
- **Average Wait Time**: How long processes wait on average
- **Average Turnaround**: Average total time in the system
- **CPU Utilization**: Percentage of time the CPU is busy
- **Throughput**: Processes completed per time unit

### Priority Boost Information
Watch the boost timer count down in real-time:
- **Last Boost**: When the last priority boost occurred
- **Next Boost**: Countdown to the next boost
- **Boost Interval**: How often boosts happen (configurable)

## Try These Experiments

### Experiment 1: Different Process Sets
- Try the "CPU-Intensive" preset to see long-running processes
- Notice how they get demoted to lower priority queues
- The "I/O-Intensive" preset shows how short processes stay in high-priority queues

### Experiment 2: Algorithm Switching
- Go to Config → Algorithm tab
- Change "Last Queue Algorithm" from Round Robin to Shortest Job First
- Run the same process set again
- See how the completion order changes!

### Experiment 3: Priority Boost
- Try turning off "Priority Boost (Aging)"
- Run a simulation with some long processes
- Notice how they might get "starved" without the boost
- Turn it back on and see the difference

### Experiment 4: Speed Control
- Use the speed slider to slow things down
- At 1x speed, you can really see what's happening step by step
- At 10x speed, you can run longer simulations quickly

### Experiment 5: Step-by-Step Mode
- Use the "⏭️ Step" button to advance one time unit at a time
- Perfect for understanding exactly how the algorithm makes decisions
- Watch processes move between queues with precise timing

## Terminal Interface (For the Curious)

If you want to dig deeper, try the terminal interface:

1. Run `./mlfq_scheduler` again
2. Choose option 1 (Run with example process set)
3. Choose option 1 (Interactive mode)
4. Press Enter to step through each time unit
5. Watch the detailed output - it shows exactly what the scheduler is thinking

This is great for understanding the algorithm, but the web interface is much prettier!

## Common Questions

**Q: The web page won't load**
A: Check if something else is using port 8080. You can change the port in the source code if needed.

**Q: Processes seem to complete in weird orders**
A: That's probably correct! MLFQ prioritizes interactive (short) processes over CPU-intensive (long) ones.

**Q: What's this "boost priority" thing?**
A: It prevents starvation. Every so often, all processes get moved back to the highest priority queue for a "second chance."

**Q: Can I create my own processes?**
A: Absolutely! Use the "➕ Add Process" button or "🎲 Random Processes" to create your own scenarios.

**Q: The numbers don't match my textbook**
A: Different textbooks use slightly different MLFQ variants. This implementation follows the classic Silberschatz approach.

**Q: Why do I see process IDs in the queues?**
A: This is a recent improvement! Instead of just showing "3 processes", you now see "P1 P3 P5" so you can track individual processes.

## Next Steps

Once you're comfortable with the basics:

1. **Try the algorithm comparison tool** (option 10/11 in the main menu)
2. **Experiment with different queue configurations**
3. **Create challenging process sets** to test edge cases
4. **Look at the source code** - it's well-commented and educational

## Need Help?

If something's not working or you're confused about the results, don't hesitate to ask. MLFQ can be tricky to understand at first, but once it clicks, it's really elegant.

The most important thing is to experiment and have fun with it. That's how I learned the most while building this!

---

*Remember: The best way to understand scheduling algorithms is to watch them work. Happy experimenting!*
