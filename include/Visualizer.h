#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "MLFQScheduler.h"
#include <string>
using namespace std;

class Visualizer 
{
private:
    MLFQScheduler& scheduler;
    int displayWidth;
    
    // Helper methods
    string getStateSymbol(ProcessState state) const;
    string getProgressBar(int current, int total, int width) const;
    
public:
    Visualizer(MLFQScheduler& sched, int width = 80);

    // Display methods
    void displayQueues() const;
    void displayCurrentProcess() const;
    void displayStats() const;
    void displayGanttChart() const;
    void displayAlgorithmInfo() const;
    void displayProcessTable() const;  // New: Show all processes info
    void displayAll() const;
    void displayAllWithProcessInfo() const;  // New: Include process table

    // Clear screen
    void clearScreen() const;
};

#endif // VISUALIZER_H
