#include "Visualizer.h"
#include "TerminalUI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <vector>
using namespace std;

using namespace TerminalUI;

Visualizer::Visualizer(MLFQScheduler& sched, int width)
    : scheduler(sched), displayWidth(width) {}

string Visualizer::getStateSymbol(ProcessState state) const
{
    switch (state)
    {
        case ProcessState::NEW: return "N";
        case ProcessState::READY: return "R";
        case ProcessState::RUNNING: return "X";
        case ProcessState::WAITING: return "W";
        case ProcessState::TERMINATED: return "T";
        default: return "?";
    }
}

string Visualizer::getProgressBar(int current, int total, int width) const
{
    return TerminalUI::progressBar(current, total, width);
}

void Visualizer::clearScreen() const
{
    TerminalUI::clearScreen();
}

void Visualizer::displayQueues() const
{
    TerminalUI::drawHeader("MULTILEVEL FEEDBACK QUEUES", displayWidth);
    
    const auto& queues = scheduler.getQueues();
    
    for (size_t i = 0; i < queues.size(); i++)
    {
        const auto& queue = queues[i];
        
        // Different colors for different queue levels
        string color;
        if (i == 0) color = Colors::SUCCESS;  // Highest priority
        else if (i == queues.size() - 1) color = Colors::ERROR;  // Lowest priority
        else color = Colors::WARNING;  // Middle priorities
        
        stringstream header;
        header << "Queue " << i << " (Quantum: " << queue.getTimeQuantum() << "ms) ["
               << queue.size() << " process" << (queue.size() != 1 ? "es" : "") << "]";
        
        cout << " " << Style::header(header.str()) << endl;
        
        if (queue.isEmpty())
        {
            cout << "    (empty)" << endl;
        }
        else
        {
            const auto& processes = queue.getProcesses();
            cout << "    ";
            int count = 0;
            for (const auto& process : processes) {
                cout << Style::highlight("P" + to_string(process->getPid())) << " ";
                count++;
                if (count >= 10) {  // Limit display
                    if (processes.size() > 10) {
                        cout << "...";
                    }
                    break;
                }
            }
            cout << endl;
        }
        
        if (i < queues.size() - 1) {
            TerminalUI::drawRowSeparator(displayWidth);
        }
    }
}

void Visualizer::displayCurrentProcess() const {
    TerminalUI::drawHeader("CURRENT EXECUTION", displayWidth);
    
    cout << " Time: " << Style::highlight(to_string(scheduler.getCurrentTime()) + " ms") << endl;
    TerminalUI::drawRowSeparator(displayWidth);
    
    auto current = scheduler.getCurrentProcess();
    if (current && current->getState() == ProcessState::RUNNING)
    {
        // Process info
        stringstream pInfo;
        pInfo << "▶ Running: " << Style::highlight("P" + to_string(current->getPid()))
              << " | Arr: " << current->getArrivalTime()
              << " | Burst: " << current->getBurstTime()
              << " | Remaining: " << Style::highlight(to_string(current->getRemainingTime()));
        
        cout << " " << pInfo.str() << endl;
        
        // Progress bar
        int progress = current->getBurstTime() - current->getRemainingTime();
        float percentage = 100.0f * progress / current->getBurstTime();
        
        cout << " Progress: " << getProgressBar(progress, current->getBurstTime(), 40) 
                  << " (" << static_cast<int>(percentage) << "%)" << endl;
    }
    else
    {
        cout << " ○ CPU IDLE" << endl;
    }
}

void Visualizer::displayStats() const {
    auto stats = scheduler.getStats();
    TerminalUI::drawHeader("PERFORMANCE STATISTICS", displayWidth);
    
    vector<pair<string, string>> statsList = {
        {"Current Time", to_string(stats.currentTime) + " ms"},
        {"Total Processes", to_string(stats.totalProcesses)},
        {"Completed Processes", to_string(stats.completedProcesses)},
        {"Avg Wait Time", to_string(stats.avgWaitTime).substr(0, 5) + " ms"},
        {"Avg Turnaround Time", to_string(stats.avgTurnaroundTime).substr(0, 5) + " ms"},
        {"Avg Response Time", to_string(stats.avgResponseTime).substr(0, 5) + " ms"},
        {"CPU Utilization", to_string(stats.cpuUtilization).substr(0, 5) + "%"}
    };
    
    for (const auto& stat : statsList) {
        cout << " " << left << setw(30) << stat.first 
                  << ": " << stat.second << endl;
    }
}

void Visualizer::displayGanttChart() const
{
    const auto& completed = scheduler.getCompletedProcesses();
    
    if (completed.empty()) return;
    
    TerminalUI::drawHeader("PROCESS COMPLETION SUMMARY", displayWidth);
    
    // Create column headers
    vector<string> headers = {"PID", "Arrival", "Burst", "Complete", "TAT", "Wait"};
    vector<int> widths = {8, 10, 10, 12, 8, 8};
    
    cout << " " << TerminalUI::formatColumn(headers, widths) << endl;
    TerminalUI::drawRowSeparator(displayWidth);
    
    // Process rows
    for (const auto& process : completed)
    {
        vector<string> row = {
            "P" + to_string(process->getPid()),
            to_string(process->getArrivalTime()) + " ms",
            to_string(process->getBurstTime()) + " ms",
            to_string(process->getCompletionTime()) + " ms",
            to_string(process->getTurnaroundTime()),
            to_string(process->getWaitTime())
        };
        
        cout << " " << TerminalUI::formatColumn(row, widths) << endl;
    }
}

void Visualizer::displayAlgorithmInfo() const
{
    string algorithmName;
    string algorithmDesc;

    switch(scheduler.getLastQueueAlgorithm())
    {
        case LastQueueAlgorithm::ROUND_ROBIN:
            algorithmName = "Round Robin (RR)";
            algorithmDesc = "Fair time-sharing, equal CPU time distribution";
            break;
        case LastQueueAlgorithm::SHORTEST_JOB_FIRST:
            algorithmName = "Shortest Job First (SJF)";
            algorithmDesc = "Shortest remaining time prioritized";
            break;
        case LastQueueAlgorithm::PRIORITY_SCHEDULING:
            algorithmName = "Priority Scheduling";
            algorithmDesc = "Longest waiting time prioritized";
            break;
    }

    TerminalUI::drawHeader("ALGORITHM INFORMATION", displayWidth);
    cout << " Last Queue Algorithm: " << Style::highlight(algorithmName) << endl;
    cout << " " << algorithmDesc << endl;
}

void Visualizer::displayProcessTable() const
{
    const auto& allProcs = scheduler.getAllProcesses();
    
    if (allProcs.empty()) return;
    
    TerminalUI::drawHeader("PROCESS INFORMATION", displayWidth);
    
    // Create column headers
    vector<string> headers = {"PID", "Arrival", "Burst", "Complete", "State", "Progress"};
    vector<int> widths = {8, 10, 10, 12, 15, 15};
    
    cout << " " << TerminalUI::formatColumn(headers, widths) << endl;
    TerminalUI::drawRowSeparator(displayWidth);
    
    // Process rows
    for (const auto& proc : allProcs)
    {
        string state;
        switch (proc->getState())
        {
            case ProcessState::NEW:
                state = "Not arrived";
                break;
            case ProcessState::READY:
                state = "Ready";
                break;
            case ProcessState::RUNNING:
                state = "Running";
                break;
            case ProcessState::TERMINATED:
                state = "Completed";
                break;
            default:
                state = "Unknown";
        }
        
        // Completion time - only show for completed processes
        string completion;
        if (proc->getState() == ProcessState::TERMINATED)
        {
            completion = to_string(proc->getCompletionTime()) + " ms";
        }
        else
        {
            completion = "-";  // Show '-' for non-completed processes
        }
        
        int completed = proc->getBurstTime() - proc->getRemainingTime();
        string progress = to_string(completed) + "/" + to_string(proc->getBurstTime());
        
        vector<string> row = 
        {
            "P" + to_string(proc->getPid()),
            to_string(proc->getArrivalTime()) + " ms",
            to_string(proc->getBurstTime()) + " ms",
            completion,
            state,
            progress
        };
        
        cout << " " << TerminalUI::formatColumn(row, widths) << endl;
    }
}

void Visualizer::displayAll() const
{
    clearScreen();
    displayCurrentProcess();
    displayQueues();
    displayStats();
    displayGanttChart();
    displayAlgorithmInfo();
}

void Visualizer::displayAllWithProcessInfo() const
{
    clearScreen();
    displayProcessTable();
    displayCurrentProcess();
    displayQueues();
    displayStats();
    displayAlgorithmInfo();
}