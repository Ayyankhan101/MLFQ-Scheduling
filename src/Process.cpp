#include "Process.h"
#include <sstream>
#include <iomanip>
using namespace std;

Process::Process(int id, int arrival, int burst)
    : pid(id), priority(0), arrivalTime(arrival), burstTime(burst),
      remainingTime(burst), cpuTimeUsed(0), lastRunTime(0), queueEnterTime(0),
      waitTime(0), turnaroundTime(0), responseTime(-1),
      completionTime(0), firstRun(true), state(ProcessState::NEW) {}

void Process::execute(int timeSlice, int currentTime) 
{
    if (firstRun) 
    {
        responseTime = currentTime - arrivalTime;
        firstRun = false;
    }
    
    state = ProcessState::RUNNING;
    lastRunTime = currentTime;
    
    int executionTime = min(timeSlice, remainingTime);
    remainingTime -= executionTime;
    cpuTimeUsed += executionTime;
    
    if (remainingTime <= 0) 
    {
        state = ProcessState::TERMINATED;
        // Note: completionTime will be set by scheduler after time increment
    }
    // Keep state as RUNNING if process is not terminated
    // The scheduler will set it to READY when the process is moved to a queue
}

void Process::calculateMetrics(int currentTime) 
{
    if (state == ProcessState::TERMINATED) 
    {
        if (completionTime == 0) 
        {
            completionTime = currentTime;
        }
        turnaroundTime = completionTime - arrivalTime;
        waitTime = turnaroundTime - burstTime;
    }
}

void Process::resetToHighestPriority() 
{
    priority = 0;
}


string Process::toString() const 
{
    stringstream ss;
    ss << "P" << setw(2) << setfill('0') << pid
       << " [Queue:" << priority
       << " Arr:" << setw(3) << arrivalTime
       << " Burst:" << setw(3) << burstTime
       << " Rem:" << setw(3) << remainingTime
       << " Wait:" << setw(3) << waitTime << "]";
    return ss.str();
}
