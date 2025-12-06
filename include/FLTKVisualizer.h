#ifndef FLTK_VISUALIZER_H
#define FLTK_VISUALIZER_H

#include "MLFQScheduler.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Counter.H>
#include <string>
#include <vector>
using namespace std;

class FLTKVisualizer : public Fl_Window 
{
private:
    MLFQScheduler& scheduler;
    
    // UI elements
    Fl_Box* titleBox;
    vector<Fl_Box*> queueBoxes;  // Dynamic queue displays
    Fl_Box* currentProcessBox;
    Fl_Box* statsBox;
    Fl_Box* ganttBox;
    Fl_Box* timeBox;
    
    Fl_Button* stepButton;
    Fl_Button* autoButton;
    Fl_Button* resetButton;
    Fl_Button* quitButton;
    
    Fl_Counter* speedCounter;
    
    // Store the current state as text
    vector<string> queueTexts;
    string currentProcessText;
    string statsText;
    string ganttText;
    
    // Store initial process configuration for reset
    struct ProcessConfig 
    {
        int arrivalTime;
        int burstTime;
    };
    vector<ProcessConfig> initialProcesses;
    
    // Update methods
    void updateQueueDisplays();
    void updateCurrentProcessDisplay();
    void updateStatsDisplay();
    void updateGanttDisplay();
    void updateAll();

    // Callback methods
    static void stepCallback(Fl_Widget* widget, void* data);
    static void autoCallback(Fl_Widget* widget, void* data);
    static void resetCallback(Fl_Widget* widget, void* data);
    static void quitCallback(Fl_Widget* widget, void* data);

public:
    FLTKVisualizer(MLFQScheduler& sched);
    ~FLTKVisualizer() = default;
    
    void run();  // Main GUI loop
    void updateDisplay();
    void saveInitialProcesses();  // Save processes for reset
    void restoreInitialProcesses();  // Restore processes after reset
    
    // Static callback handlers
    void handleStep();
    void handleAuto();
    void handleReset();
    void handleQuit();
};

#endif // FLTK_VISUALIZER_H