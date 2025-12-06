#ifndef MLFQ_SCHEDULER_H
#define MLFQ_SCHEDULER_H

#include "Process.h"
#include "Queue.h"
#include "SchedulerConfig.h"
#include <vector>
#include <memory>
#include <map>
using namespace std;

struct SchedulerStats 
{
    double avgWaitTime;
    double avgTurnaroundTime;
    double avgResponseTime;
    double cpuUtilization;
    int totalProcesses;
    int completedProcesses;
    int currentTime;
};

class MLFQScheduler 
{
private:
    vector<ProcessQueue> readyQueues;
    vector<shared_ptr<Process>> allProcesses;
    vector<shared_ptr<Process>> completedProcesses;
    shared_ptr<Process> currentProcess;

    int currentTime;
    int boostTimer;
    int boostInterval;  // Priority boost interval (aging prevention)
    int numQueues;
    int pidCounter;     // Process ID counter
    SchedulerConfig config;  // Configuration parameters
    LastQueueAlgorithm lastQueueAlgorithm;  // Algorithm for last queue


    // Execution log for Gantt chart
    struct ExecutionRecord
    {
        int pid;
        int startTime;
        int endTime;
        int queueLevel;
    };
    vector<ExecutionRecord> executionLog;

    // Helper methods
    void moveToNextQueue(shared_ptr<Process> process);
    void insertProcessIntoLastQueueByAlgorithm(shared_ptr<Process> process);
    void boostAllProcesses();
    void updateWaitTimes();
    shared_ptr<Process> selectNextProcess();
    shared_ptr<Process> selectNextProcessForLastQueue();  // Select based on algorithm
    void checkNewArrivals();

public:
    // Algorithm selection methods
    void setLastQueueAlgorithm(LastQueueAlgorithm algorithm);
    LastQueueAlgorithm getLastQueueAlgorithm() const { return lastQueueAlgorithm; }

    // AI-related helper methods
    void adaptiveParameterUpdate();
    void recordSchedulingDecision(shared_ptr<Process> process, int decision);

public:
    // Constructors
    MLFQScheduler(int queues = 3, int boost = 100);  // Legacy constructor
    MLFQScheduler(const SchedulerConfig& cfg);        // Configuration-based constructor

    // Destructor
    ~MLFQScheduler();

    // Process management
    void addProcess(int arrivalTime, int burstTime);
    void addProcess(shared_ptr<Process> process);

    // Scheduling
    void step();  // Execute one time unit
    bool hasProcesses() const;
    bool isComplete() const;


    // Getters
    int getCurrentTime() const { return currentTime; }
    shared_ptr<Process> getCurrentProcess() const { return currentProcess; }
    const vector<ProcessQueue>& getQueues() const { return readyQueues; }
    const vector<shared_ptr<Process>>& getAllProcesses() const { return allProcesses; }
    const vector<shared_ptr<Process>>& getCompletedProcesses() const { return completedProcesses; }

    // Statistics
    SchedulerStats getStats() const;

    // Reset
    void reset();

    // Get execution log for visualization
    const vector<ExecutionRecord>& getExecutionLog() const { return executionLog; }

    // Get configuration
    const SchedulerConfig& getConfig() const { return config; }


};

#endif // MLFQ_SCHEDULER_H
