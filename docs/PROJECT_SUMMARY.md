# MLFQ Scheduler Project - A Learning Journey

## What This Project Is About

This started as a simple assignment for my Operating Systems class, but it turned into something much bigger. I wanted to build an MLFQ (Multilevel Feedback Queue) scheduler that wasn't just functional, but actually helped people understand how this fascinating algorithm works.

## The Problem I Wanted to Solve

When I first learned about MLFQ, I found the textbook explanations pretty abstract. Sure, I could memorize the rules:
- Processes start in the highest priority queue
- If they use their full quantum, they get demoted
- Periodic boosts prevent starvation

But I wanted to *see* it happening. Most online simulators were either too simple or didn't show the details that matter. So I decided to build my own.

## What Makes This Different

### Multiple Ways to Learn
I built three different interfaces because everyone learns differently:

**Web Interface**: This is probably my favorite part. It's beautiful, runs in any browser, and shows everything in real-time. You can watch processes move between queues, see performance metrics update live, and even get visual indicators when priority boosts happen.

**Terminal Interface**: Perfect for when you want to understand exactly what's happening. You can step through the algorithm one time unit at a time and see the scheduler's decision-making process.

**FLTK GUI**: A native desktop application for those who prefer traditional interfaces. Fast and responsive for longer simulations.

### Real Algorithm Flexibility
Most MLFQ implementations just use round-robin everywhere. I implemented three different algorithms for the last queue:
- **Round Robin**: Fair time-sharing
- **Shortest Job First**: Optimizes for responsiveness  
- **Priority Scheduling**: Uses aging to prevent starvation

This flexibility makes the scheduler much more interesting and closer to real-world implementations.

### Educational Focus
Everything is designed to help you learn:
- Clear visualizations that show queue states
- Comprehensive performance metrics with proper formulas
- Step-by-step execution modes
- Algorithm comparison tools
- Well-commented source code

## Technical Achievements

### Accurate Implementation
I spent a lot of time making sure the algorithms are mathematically correct:
- Time quantum tracking is precise
- Performance metrics use standard OS formulas
- Process state transitions follow textbook specifications
- Boost priority (aging) works exactly as described in literature

### Performance Metrics
All calculations are accurate:
- **Wait Time** = Turnaround Time - Burst Time
- **Turnaround Time** = Completion Time - Arrival Time  
- **Response Time** = First CPU Access - Arrival Time
- **CPU Utilization** = (Total CPU Time / Total Elapsed Time) × 100

I double-checked these against multiple sources and test cases.

### Web-Backend Synchronization
Getting the JavaScript web interface to produce exactly the same results as the C++ backend was challenging. I had to be very careful about timing, state management, and algorithm implementation to ensure consistency.

## Features I'm Proud Of

### Boost Priority Visualization
The aging mechanism is crucial for preventing starvation, but it's often invisible in other simulators. I added visual indicators that show exactly when boosts occur and how they affect the queue states.

### Algorithm Comparison Tool
Want to see how SJF compares to Round Robin for the same process set? This feature runs identical workloads through different algorithms and shows you the results side by side. It's incredibly useful for understanding algorithm trade-offs.

### Flexible Process Generation
You can:
- Use predefined example sets (great for learning)
- Create custom process sets (test specific scenarios)
- Generate random processes (stress testing)
- Load processes while understanding their characteristics

### Real-Time Configuration
Change algorithms, adjust boost intervals, modify queue parameters - all while seeing the immediate effects on performance metrics.

## What I Learned

### Technical Skills
- **C++ Systems Programming**: Memory management, object-oriented design, performance optimization
- **Web Development**: HTML5, CSS3, JavaScript, real-time visualization
- **Algorithm Implementation**: Translating theoretical concepts into working code
- **Testing and Validation**: Ensuring correctness through comprehensive test cases

### Operating Systems Concepts
Building this forced me to understand MLFQ at a much deeper level:
- Why quantum progression matters (1, 2, 4, 8...)
- How aging prevents starvation without destroying performance
- The trade-offs between responsiveness and fairness
- Why different last-queue algorithms matter in practice

### Software Engineering
- **Documentation**: Writing for different audiences (users vs. developers)
- **User Interface Design**: Making complex algorithms accessible
- **Code Organization**: Keeping a growing codebase maintainable
- **Cross-Platform Development**: Ensuring compatibility across systems

## Challenges I Overcame

### Quantum Implementation
Getting the time quantum logic right was harder than expected. Each queue needs to track how long processes have been running, handle quantum expiration correctly, and manage transitions between queues. I probably rewrote this part three times before getting it right.

### Performance Metric Accuracy
Calculating things like wait time and turnaround time sounds simple, but there are edge cases everywhere. What happens when a process gets boosted? How do you handle processes that haven't completed yet? I ended up creating a comprehensive test suite just for the metrics.

### Visualization Synchronization
Making sure the web interface shows exactly what the C++ backend is doing required careful attention to timing and state management. Any discrepancy would confuse users and undermine the educational value.

## Impact and Usage

### Educational Value
This project has helped me and my classmates understand MLFQ much better than textbook reading alone. The visual feedback makes abstract concepts concrete, and the ability to experiment with parameters builds intuition.

### Code Quality
I tried to write code that others could learn from:
- Clear variable names and function signatures
- Comprehensive comments explaining the "why" not just the "what"
- Modular design that separates concerns
- Consistent coding style throughout

### Extensibility
The architecture makes it relatively easy to:
- Add new scheduling algorithms
- Implement different queue management policies
- Extend the visualization capabilities
- Add new performance metrics

## Future Possibilities

### Multi-Core Extension
Extending this to simulate multi-core MLFQ would be fascinating. How do you balance loads across cores? How do you handle process migration?

### Real-Time Features
Adding the ability to inject new processes while simulation is running would make it even more realistic and educational.

### Advanced Visualizations
Different chart types, export capabilities, and mobile-friendly interfaces could make it accessible to even more learners.

## Personal Reflection

This project taught me more about operating systems than any textbook ever could. There's something about implementing an algorithm from scratch that forces you to understand every detail, every edge case, every design decision.

I'm particularly proud of how it turned out as an educational tool. When my classmates use it and suddenly understand why MLFQ works the way it does, or when they discover something new by experimenting with the parameters - that's when I know the effort was worth it.

## Technical Specifications

### Architecture
- **Core Engine**: C++ with object-oriented design
- **Web Interface**: HTML5/CSS3/JavaScript with real-time updates
- **Desktop GUI**: FLTK for cross-platform compatibility
- **Build System**: CMake with automated dependency management

### Performance
- Handles dozens of processes without performance issues
- Real-time visualization at multiple speed settings
- Efficient memory management with smart pointers
- Optimized for educational use rather than production scale

### Compatibility
- Linux (primary development platform)
- macOS (tested and working)
- Windows (should work with minor modifications)
- Any modern web browser for the web interface

## Acknowledgments

This project was inspired by the classic MLFQ descriptions in operating systems textbooks, particularly Silberschatz's "Operating System Concepts." The implementation is my own work, but the algorithm itself comes from decades of operating systems research.

Thanks to my OS professor for encouraging this kind of deep dive into the material, and to my classmates who provided feedback and helped me test edge cases.

## Final Thoughts

What started as a class assignment became a genuine passion project. I hope it's as useful for learning as it was for building. MLFQ is a beautiful algorithm - elegant, practical, and surprisingly deep. This implementation tries to do justice to that elegance while making it accessible to students.

If you use this project and learn something new about operating systems, then I've accomplished what I set out to do.

---

*"The best way to understand an algorithm is to implement it yourself."*
*- Every CS professor ever (and they're absolutely right)*
