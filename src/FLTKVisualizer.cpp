#include "FLTKVisualizer.h"
#include <sstream>
#include <iomanip>
#include <string>
using namespace std;

// Static callback implementations
void FLTKVisualizer::stepCallback(Fl_Widget* /*widget*/, void* data)
{
    FLTKVisualizer* viz = static_cast<FLTKVisualizer*>(data);
    viz->handleStep();
}

void FLTKVisualizer::autoCallback(Fl_Widget* /*widget*/, void* data)
{
    FLTKVisualizer* viz = static_cast<FLTKVisualizer*>(data);
    viz->handleAuto();
}

void FLTKVisualizer::resetCallback(Fl_Widget* /*widget*/, void* data)
{
    FLTKVisualizer* viz = static_cast<FLTKVisualizer*>(data);
    viz->handleReset();
}

void FLTKVisualizer::quitCallback(Fl_Widget* /*widget*/, void* data)
{
    FLTKVisualizer* viz = static_cast<FLTKVisualizer*>(data);
    viz->handleQuit();
}

FLTKVisualizer::FLTKVisualizer(MLFQScheduler& sched)
    : Fl_Window(700, 750, "MLFQ Scheduler Visualization"), scheduler(sched)
    {
    // Save initial processes for reset functionality
    saveInitialProcesses();

    // Get number of queues from scheduler configuration
    const int totalQueues = scheduler.getConfig().numQueues;

    // Resize text vectors
    queueTexts.resize(totalQueues);
    queueBoxes.resize(totalQueues);

    // Set window properties
    this->begin();

    // Window padding and dimensions
    const int PADDING = 10;
    const int WIN_WIDTH = 700;
    const int CONTENT_WIDTH = WIN_WIDTH - (2 * PADDING);

    int currentY = PADDING;

    // Create title bar area
    titleBox = new Fl_Box(PADDING, currentY, CONTENT_WIDTH - 100, 35, "MLFQ Scheduler");
    titleBox->labelsize(16);
    titleBox->labelfont(FL_BOLD);
    titleBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    titleBox->box(FL_FLAT_BOX);
    titleBox->color(FL_DARK_BLUE);
    titleBox->labelcolor(FL_WHITE);

    // Create time display (top right)
    timeBox = new Fl_Box(WIN_WIDTH - 110, currentY, 100, 35, "Time: 0");
    timeBox->box(FL_FLAT_BOX);
    timeBox->color(FL_DARK_BLUE);
    timeBox->labelcolor(FL_WHITE);
    timeBox->labelsize(14);
    timeBox->labelfont(FL_BOLD);
    timeBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

    currentY += 40;

    // Create current process display
    currentProcessBox = new Fl_Box(PADDING, currentY, CONTENT_WIDTH, 50, "Current Process");
    currentProcessBox->box(FL_BORDER_BOX);
    currentProcessBox->color(fl_rgb_color(200, 255, 255));  // Light cyan
    currentProcessBox->labelcolor(FL_BLACK);
    currentProcessBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
    currentProcessBox->labelsize(12);
    currentProcessBox->labelfont(FL_COURIER_BOLD);
    currentY += 60;

    // Create queue displays with consistent spacing
    // Adjust queue height based on number of queues to fit in window
    const int QUEUE_SPACING = 6;
    const int AVAILABLE_SPACE = 350;  // Increased space for queues since we have more room
    const int QUEUE_HEIGHT = (AVAILABLE_SPACE - (totalQueues - 1) * QUEUE_SPACING) / totalQueues;

    // Define colors for up to 5 queues
    const Fl_Color queueColors[] = {
        fl_rgb_color(200, 220, 255),  // Light blue
        fl_rgb_color(200, 255, 200),  // Light green
        fl_rgb_color(255, 255, 200),  // Light yellow
        fl_rgb_color(255, 220, 220),  // Light red
        fl_rgb_color(230, 220, 255)   // Light purple
    };

    for (int i = 0; i < totalQueues; i++)
    {
        queueBoxes[i] = new Fl_Box(PADDING, currentY, CONTENT_WIDTH, QUEUE_HEIGHT, "");
        queueBoxes[i]->box(FL_BORDER_BOX);
        queueBoxes[i]->color(queueColors[i % 5]);
        queueBoxes[i]->labelcolor(FL_BLACK);
        queueBoxes[i]->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
        queueBoxes[i]->labelsize(11);
        queueBoxes[i]->labelfont(FL_COURIER_BOLD);

        currentY += QUEUE_HEIGHT + QUEUE_SPACING;
    }

    // Create stats display
    statsBox = new Fl_Box(PADDING, currentY, CONTENT_WIDTH, 80, "Statistics");
    statsBox->box(FL_BORDER_BOX);
    statsBox->color(fl_rgb_color(240, 240, 240));  // Light gray
    statsBox->labelcolor(FL_BLACK);
    statsBox->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
    statsBox->labelsize(10);
    statsBox->labelfont(FL_COURIER_BOLD);
    currentY += 90;

    // Create Process Information display (was Gantt chart)
    ganttBox = new Fl_Box(PADDING, currentY, CONTENT_WIDTH, 110, "Process Info");
    ganttBox->box(FL_BORDER_BOX);
    ganttBox->color(fl_rgb_color(255, 250, 240));  // Light cream
    ganttBox->labelcolor(FL_BLACK);
    ganttBox->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
    ganttBox->labelsize(10);
    ganttBox->labelfont(FL_COURIER_BOLD);
    currentY += 120;

    // Create control buttons with proper spacing
    const int BUTTON_WIDTH = 85;
    const int BUTTON_HEIGHT = 30;
    const int BUTTON_SPACING = 10;
    int buttonX = PADDING;

    stepButton = new Fl_Button(buttonX, currentY, BUTTON_WIDTH, BUTTON_HEIGHT, "Step");
    stepButton->callback(stepCallback, this);
    stepButton->color(fl_rgb_color(200, 255, 250));
    buttonX += BUTTON_WIDTH + BUTTON_SPACING ;

    autoButton = new Fl_Button(buttonX, currentY, BUTTON_WIDTH, BUTTON_HEIGHT, "Auto");
    autoButton->callback(autoCallback, this);
    autoButton->color(FL_LIGHT2);
    buttonX += BUTTON_WIDTH + BUTTON_SPACING;

    resetButton = new Fl_Button(buttonX, currentY, BUTTON_WIDTH, BUTTON_HEIGHT, "Reset");
    resetButton->callback(resetCallback, this);
    resetButton->color(fl_rgb_color(255, 200, 200));
    buttonX += BUTTON_WIDTH + BUTTON_SPACING;

    quitButton = new Fl_Button(buttonX, currentY, BUTTON_WIDTH, BUTTON_HEIGHT, "Quit");
    quitButton->callback(quitCallback, this);
    quitButton->color(fl_rgb_color(255, 200, 200));  // Light red
    buttonX += BUTTON_WIDTH + BUTTON_SPACING + 20;

    // Speed control with label
    Fl_Box* speedLabel = new Fl_Box(buttonX, currentY, 60, BUTTON_HEIGHT, "Speed:");
    speedLabel->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    speedLabel->labelsize(12);
    buttonX += 65;

    speedCounter = new Fl_Counter(buttonX, currentY, 90, BUTTON_HEIGHT);
    speedCounter->range(0.1, 5.0);
    speedCounter->value(1.0);
    speedCounter->step(0.1);
    speedCounter->lstep(0.5);

    this->end();
    this->resizable(this);
    this->size_range(700, 750, 0, 0);  // Set minimum size

    // Initialize text
    updateAll();
}

void FLTKVisualizer::updateQueueDisplays() 
{
    const auto& queues = scheduler.getQueues();
    auto current = scheduler.getCurrentProcess();
    
    for (size_t i = 0; i < queues.size() && i < queueBoxes.size(); i++) 
    {
        ostringstream oss;
        oss << "Queue " << i << " (Quantum: " << queues[i].getTimeQuantum() 
            << "ms) [" << queues[i].size() << " processes ]";
        
        // Add CPU indicator if current process belongs to this queue
        if (current && current->getPriority() == static_cast<int>(i) && 
            current->getState() != ProcessState::TERMINATED) 
        {
            oss << " → CPU";
        }
        oss << "\n";
        
        const auto& processes = queues[i].getProcesses();
        int count = 0;
        size_t maxDisplay = (queueBoxes.size() <= 3) ? 4 : 3;  // Adjust for more queues
        size_t processSize = processes.size();

        for (const auto& process : processes)
        {
            if (count >= static_cast<int>(maxDisplay))
            {  // Limit display to prevent overflow
                oss << "... and " << (processSize - maxDisplay) << " more";
                break;
            }

            int progress = process->getBurstTime() - process->getRemainingTime();
            double percent = (process->getBurstTime() > 0) ?
                (static_cast<double>(progress) * 100.0 / process->getBurstTime()) : 0.0;

            oss << "  P" << setw(2) << setfill('0') << process->getPid()
                << " [" << setw(2) << progress << "/" << setw(2)
                << process->getBurstTime() << "] "
                << fixed << setprecision(0) << setw(3) << percent << "%";

            if (count < static_cast<int>(processSize - 1) && count < static_cast<int>(maxDisplay - 1))
            {
                oss << "\n";
            }
            count++;
        }
        
        if (processes.empty()) 
        {
            oss << "  (empty)";
        }
        
        queueTexts[i] = oss.str();
        queueBoxes[i]->copy_label(queueTexts[i].c_str());
    }
    
    // Refresh the display
    for (size_t i = 0; i < queueBoxes.size(); i++) 
    {
        queueBoxes[i]->redraw();
    }
}

void FLTKVisualizer::updateCurrentProcessDisplay() 
{
    ostringstream oss;
    auto current = scheduler.getCurrentProcess();
    
    if (current && current->getState() != ProcessState::TERMINATED) 
    {
        int progress = current->getBurstTime() - current->getRemainingTime();
        double percent = (current->getBurstTime() > 0) ? 
            (static_cast<double>(progress) * 100.0 / current->getBurstTime()) : 0.0;
        
        // Show as RUNNING when there's a current process that isn't terminated
        oss << "RUNNING: P" << setw(2) << setfill('0') << current->getPid()
            << " | Queue: " << current->getPriority()
            << " | Progress: " << progress << "/" << current->getBurstTime()
            << " (" << fixed << setprecision(1) << percent << "%)";
    }
     else 
     {
        oss << "CPU IDLE";
    }
    
    currentProcessText = oss.str();
    currentProcessBox->copy_label(currentProcessText.c_str());
    currentProcessBox->redraw();
}

void FLTKVisualizer::updateStatsDisplay() 
{
    auto stats = scheduler.getStats();
    ostringstream oss;
    
    oss << "Statistics Summary:\n"
        << "  Time: " << stats.currentTime << " ms    "
        << "Processes: " << stats.completedProcesses << "/" << stats.totalProcesses << "\n"
        << "  Avg Wait: " << fixed << setprecision(1) << stats.avgWaitTime << " ms  "
        << "Avg Turnaround: " << stats.avgTurnaroundTime << " ms\n"
        << "  Avg Response: " << stats.avgResponseTime << " ms    "
        << "CPU Util: " << setprecision(1) << stats.cpuUtilization << "%";
    
    statsText = oss.str();
    statsBox->copy_label(statsText.c_str());
    statsBox->redraw();
}

void FLTKVisualizer::updateGanttDisplay()
{
    // Display process information table in a compact format
    ostringstream oss;
    oss << "Process Information:\n";

    const auto& allProcs = scheduler.getAllProcesses();

    if (allProcs.empty())
    {
        oss << " No processes loaded.";
    }
    else
    {
        // Compact header that fits in the box (680px width) - Added completion column
        oss << " ID | Arr | Bur | Comp | State   | Prog\n";
        oss << " " << string(41, '-') << "\n";

        for (const auto& proc : allProcs)
        {
            // Compact format: P01 | 0 | 20 | 25 | Running | 5/20
            oss << " P" << setw(2) << setfill('0') << proc->getPid() << " |";
            oss << setw(3) << setfill(' ') << proc->getArrivalTime() << " |";
            oss << setw(3) << proc->getBurstTime() << " |";

            // Completion time - only show for completed processes
            if (proc->getState() == ProcessState::TERMINATED)
            {
                oss << setw(4) << proc->getCompletionTime() << " |";
            }
            else
            {
                oss << "  - |";  // Show '-' for non-completed processes
            }

            // State - abbreviated to fit
            string state;
            switch (proc->getState())
            {
                case ProcessState::NEW:
                    state = "NotYet";
                    break;
                case ProcessState::READY:
                    state = "Ready";
                    break;
                case ProcessState::RUNNING:
                    state = "Running";
                    break;
                case ProcessState::TERMINATED:
                    state = "Done";
                    break;
                default:
                    state = "Unknown";
            }
            oss << " " << left << setw(7) << state << " |";

            // Progress - compact format
            int completed = proc->getBurstTime() - proc->getRemainingTime();
            oss << " " << setw(2) << completed << "/" << setw(2) << proc->getBurstTime();

            if (proc != allProcs.back())
            {
                oss << "\n";
            }
        }
    }

    ganttText = oss.str();
    ganttBox->copy_label(ganttText.c_str());
    ganttBox->redraw();
}

void FLTKVisualizer::updateAll() 
{
    updateQueueDisplays();
    updateCurrentProcessDisplay();
    updateStatsDisplay();
    updateGanttDisplay();
    
    // Update time display
    ostringstream timeOss;
    timeOss << "Time: " << scheduler.getCurrentTime();
    timeBox->copy_label(timeOss.str().c_str());
    timeBox->redraw();
    
    this->redraw();
}

void FLTKVisualizer::updateDisplay() 
{
    updateAll();
    Fl::check();  // Process events but don't block
}

void FLTKVisualizer::run() 
{
    this->show();
    
    // Main event loop
    while(this->shown()) 
    {
        if (Fl::wait(0.03) > 0) 
        {  // Wait up to 30ms for events
            Fl::check();
        }
        Fl::flush();  // Process any pending events
    }
}

void FLTKVisualizer::handleStep() 
{
    if (!scheduler.isComplete()) 
    {
        scheduler.step();
        updateDisplay();
    }
}

void FLTKVisualizer::handleAuto() 
{
    // Toggle auto mode with timer
    if (!scheduler.isComplete()) 
    {
        // Run multiple steps with delays
        for (int i = 0; i < 50 && !scheduler.isComplete(); i++) 
        {
            scheduler.step();
            updateDisplay();
            Fl::wait(0.2 / speedCounter->value());  // Adjustable speed
        }
    }
}

void FLTKVisualizer::handleReset() 
{
    // Reset scheduler (clears all processes)
    scheduler.reset();
    
    // Restore the initial processes
    restoreInitialProcesses();
    
    // Update display to show reset state
    updateDisplay();
}

void FLTKVisualizer::saveInitialProcesses() 
{
    // Save the current processes configuration
    initialProcesses.clear();
    const auto& allProcs = scheduler.getAllProcesses();
    
    for (const auto& proc : allProcs) 
    {
        ProcessConfig config;
        config.arrivalTime = proc->getArrivalTime();
        config.burstTime = proc->getBurstTime();
        initialProcesses.push_back(config);
    }
}

void FLTKVisualizer::restoreInitialProcesses() 
{
    // Re-add all the initial processes to the scheduler
    for (const auto& config : initialProcesses) 
    {
        scheduler.addProcess(config.arrivalTime, config.burstTime);
    }
}

void FLTKVisualizer::handleQuit() 
{
    this->hide();
}