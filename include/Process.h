#ifndef PROCESS_H
#define PROCESS_H

#include <string>
using namespace std;

enum class ProcessState 
{
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

class Process 
{
private:
    int pid;
    int priority;           // queue level (0 = highest)
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int cpuTimeUsed;
    int lastRunTime;
    int queueEnterTime;     // when process entered current queue
    int waitTime;
    int turnaroundTime;
    int responseTime;
    int completionTime;
    bool firstRun;
    ProcessState state;

public:
    // Constructor
    Process(int id, int arrival, int burst);
    
    // Getters
    int getPid() const { return pid; }
    int getPriority() const { return priority; }
    int getArrivalTime() const { return arrivalTime; }
    int getBurstTime() const { return burstTime; }
    int getRemainingTime() const { return remainingTime; }
    int getCpuTimeUsed() const { return cpuTimeUsed; }
    int getLastRunTime() const { return lastRunTime; }
    int getQueueEnterTime() const { return queueEnterTime; }
    int getWaitTime() const { return waitTime; }
    int getTurnaroundTime() const { return turnaroundTime; }
    int getResponseTime() const { return responseTime; }
    int getCompletionTime() const { return completionTime; }
    ProcessState getState() const { return state; }
    bool isFirstRun() const { return firstRun; }
    
    // Setters
    void setPriority(int p) { priority = p; }
    void setState(ProcessState s) { state = s; }
    void setLastRunTime(int t) { lastRunTime = t; }
    void setQueueEnterTime(int t) { queueEnterTime = t; }
    void setCompletionTime(int t) { completionTime = t; }
    
    // Operations
    void execute(int timeSlice, int currentTime);
    void incrementWaitTime() { waitTime++; }
    void calculateMetrics(int currentTime);
    void resetToHighestPriority();
    void reset();  // Reset process to initial state
    
    // Display
    string toString() const;
};

#endif // PROCESS_H
