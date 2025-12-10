#include "MLFQScheduler.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <climits>
using namespace std;

// Legacy constructor
MLFQScheduler::MLFQScheduler(int queues, int boost)
    : currentProcess(nullptr), currentTime(0), boostTimer(0), boostInterval(boost), numQueues(queues),
      pidCounter(1), lastQueueAlgorithm(LastQueueAlgorithm::ROUND_ROBIN) 
{

    // Create default config
    config = SchedulerConfig();
    config.numQueues = queues;
    config.boostInterval = boost;

    // Initialize queues with different time quantums
    int baseQuantum = config.baseQuantum;

    for (int i = 0; i < numQueues; i++) 
    {
        int quantum = static_cast<int>(baseQuantum * pow(config.quantumMultiplier, i));
        readyQueues.emplace_back(i, quantum);
    }
}

// Configuration-based constructor
MLFQScheduler::MLFQScheduler(const SchedulerConfig& cfg)
    : currentProcess(nullptr), currentTime(0), boostTimer(0),
      boostInterval(cfg.boostInterval), numQueues(cfg.numQueues), pidCounter(1), config(cfg),
      lastQueueAlgorithm(LastQueueAlgorithm::ROUND_ROBIN) 
{

    // Initialize queues using configuration
    for (int i = 0; i < numQueues; i++) 
    {
        int quantum = config.getQuantumForQueue(i);
        readyQueues.emplace_back(i, quantum);
    }
}

void MLFQScheduler::addProcess(int arrivalTime, int burstTime) 
{
    auto process = make_shared<Process>(pidCounter++, arrivalTime, burstTime);
    allProcesses.push_back(process);
}

void MLFQScheduler::addProcess(shared_ptr<Process> process) 
{
    if (process) 
    {
        allProcesses.push_back(process);
    }
}

void MLFQScheduler::checkNewArrivals() 
{
    for (auto& process : allProcesses) 
    {
        if (process->getArrivalTime() == currentTime && process->getState() == ProcessState::NEW) 
        {
            process->setState(ProcessState::READY);  // Set to READY when entering queue
            process->setQueueEnterTime(currentTime);
            readyQueues[0].enqueue(process);  // New processes start at highest priority
        }
    }
}

shared_ptr<Process> MLFQScheduler::selectNextProcess() 
{
    // Rule 1: If priority(A) > priority(B), A runs before B
    const size_t queueCount = readyQueues.size();
    for (size_t i = 0; i < queueCount; i++)
    {
        if (!readyQueues[i].isEmpty())
        {
            // For the last queue, use the selected algorithm
            if (i == queueCount - 1)
            {  // Last queue
                return selectNextProcessForLastQueue();
            }
             else
             {
                // For other queues, use standard round-robin (FIFO)
                auto process = readyQueues[i].dequeue();
                return process;
            }
        }
    }
    return nullptr;
}

shared_ptr<Process> MLFQScheduler::selectNextProcessForLastQueue()
{
    if (readyQueues.empty()) return nullptr;

    int lastQueueIndex = readyQueues.size() - 1;
    auto& lastQueue = readyQueues[lastQueueIndex];

    if (lastQueue.isEmpty()) return nullptr;

    shared_ptr<Process> selectedProcess = nullptr;

    switch (lastQueueAlgorithm)
    {
        case LastQueueAlgorithm::ROUND_ROBIN:
        {
            // Standard round-robin: take first process (FIFO), so we dequeue it
            return lastQueue.dequeue();  // This removes and returns the first process
        }

        case LastQueueAlgorithm::SHORTEST_JOB_FIRST:
        {
            // Find process with shortest remaining time
            auto processes = lastQueue.getProcesses(); // Get a copy to sort
            // Sort by remaining time (shortest first)
            sort(processes.begin(), processes.end(),
                [](const shared_ptr<Process>& a, const shared_ptr<Process>& b) 
                {
                    return a->getRemainingTime() < b->getRemainingTime();
                });

            // The first process in the sorted list is the one to execute
            selectedProcess = processes[0];
            break;
        }

        case LastQueueAlgorithm::PRIORITY_SCHEDULING:
        {
            // Find the highest priority process (longest waiting time)
            auto processes = lastQueue.getProcesses(); // Get a copy to sort
            // Sort by priority (longest waiting time first)
            sort(processes.begin(), processes.end(),
                [](const shared_ptr<Process>& a, const shared_ptr<Process>& b) 
                {
                    // Priority = actual wait time (aging for anti-starvation)
                    return a->getWaitTime() > b->getWaitTime(); // Higher wait time first
                });

            // The first process in the sorted list is the one to execute
            selectedProcess = processes[0];
            break;
        }
    }

    // For SJF and Priority Scheduling, remove the selected process to return it
    if (selectedProcess)
    {
        lastQueue.removeProcess(selectedProcess->getPid());
    }
    return selectedProcess;
}

void MLFQScheduler::setLastQueueAlgorithm(LastQueueAlgorithm algorithm)
{
    lastQueueAlgorithm = algorithm;
}


void MLFQScheduler::moveToNextQueue(shared_ptr<Process> process)
{
    if (!process || process->getState() == ProcessState::TERMINATED)
    {
        return;
    }

    // Rule 3: When a process uses up its time slice, move to lower priority queue
    int currentPriority = process->getPriority();
    int nextPriority = min(currentPriority + 1, numQueues - 1);

    process->setPriority(nextPriority);
    process->setState(ProcessState::READY);  // Set to READY when moved to queue
    process->setQueueEnterTime(currentTime);

    // For SJF and Priority Scheduling in the last queue, we need special handling
    if (nextPriority == numQueues - 1 && lastQueueAlgorithm != LastQueueAlgorithm::ROUND_ROBIN)
    {
        // Instead of using a separate method, let's insert the process in the correct position directly
        insertProcessIntoLastQueueByAlgorithm(process);
    }
    else
    {
        readyQueues[nextPriority].enqueue(process);
    }
}

void MLFQScheduler::insertProcessIntoLastQueueByAlgorithm(shared_ptr<Process> process)
{
    if (!process) return;

    int lastQueueIndex = numQueues - 1;
    auto& lastQueue = readyQueues[lastQueueIndex];

    // Get the current list of processes in the last queue
    auto processes = lastQueue.getProcesses();

    // Find the correct insertion point based on the algorithm
    switch (lastQueueAlgorithm)
    {
        case LastQueueAlgorithm::SHORTEST_JOB_FIRST:
        {
            // Find where to insert based on remaining time (shortest first)
            auto insertPos = processes.begin();
            for (; insertPos != processes.end(); ++insertPos) 
            {
                if ((*insertPos)->getRemainingTime() > process->getRemainingTime()) 
                {
                    break;
                }
            }
            processes.insert(insertPos, process);
            break;
        }
        case LastQueueAlgorithm::PRIORITY_SCHEDULING:
        {
            // Find where to insert based on priority (longest waiting time first)
            int processPriority = currentTime - process->getArrivalTime();
            auto insertPos = processes.begin();
            for (; insertPos != processes.end(); ++insertPos) 
            {
                int currentPriority = currentTime - (*insertPos)->getArrivalTime();
                if (currentPriority < processPriority) 
                {  // Higher priority value comes first
                    break;
                }
            }
            processes.insert(insertPos, process);
            break;
        }
        default:
            // Round Robin is handled by regular enqueue
            lastQueue.enqueue(process);
            return;
    }

    // Clear and repopulate the queue in the correct order
    lastQueue.clear();
    for (const auto& proc : processes)
    {
        lastQueue.enqueue(proc);
    }
}

void MLFQScheduler::boostAllProcesses() 
{
    // Rule 4: After time period S, move all processes to highest priority queue
    for (int i = 1; i < numQueues; i++) 
    {
        auto& queue = readyQueues[i];
        while (!queue.isEmpty()) 
        {
            auto process = queue.dequeue();
            if (process && process->getState() != ProcessState::TERMINATED) 
            {
                process->resetToHighestPriority();
                process->setState(ProcessState::READY);  // Set to READY when boosted
                readyQueues[0].enqueue(process);
            }
        }
    }
}

void MLFQScheduler::updateWaitTimes() 
{
    for (auto& queue : readyQueues) 
    {
        for (const auto& process : queue.getProcesses()) 
        {
            if (process->getState() == ProcessState::READY) 
            {
                process->incrementWaitTime();
            }
        }
    }
}

void MLFQScheduler::step() 
{
    // Check for new arrivals
    checkNewArrivals();

    // If no current process, select next one
    if (!currentProcess || currentProcess->getState() == ProcessState::TERMINATED) 
    {
        currentProcess = selectNextProcess();
        if (!currentProcess) 
        {
            currentTime++;
            return;  // Idle time - don't count boost timer during idle
        }
        // Set queue enter time only when newly selected
        currentProcess->setQueueEnterTime(currentTime);
    }

    // Execute current process
    int queueLevel = currentProcess->getPriority();
    int timeQuantum = readyQueues[queueLevel].getTimeQuantum();
    int timeSlice = 1;  // Execute 1 time unit at a time for visualization

    int executionStart = currentTime;
    currentProcess->execute(timeSlice, currentTime);

    // Log execution
    executionLog.push_back(
    {
        currentProcess->getPid(),
        executionStart,
        currentTime + timeSlice,
        queueLevel
    });

    // Update time first (before checking completion)
    currentTime++;

    // Update wait times for other processes AFTER time increment
    updateWaitTimes();

    // Calculate quantum usage and check completion status AFTER execution
    int timeUsedInQueue = currentTime - currentProcess->getQueueEnterTime();  // This now correctly measures time in queue
    bool quantumExpired = (timeUsedInQueue >= timeQuantum);
    bool processTerminated = (currentProcess->getState() == ProcessState::TERMINATED);  // This checks status after execution

    // Priority boost check - only increment when CPU is active
    boostTimer++;
    if (boostTimer >= boostInterval)
    {
        boostAllProcesses();
        boostTimer = 0;
        // Current process also gets boosted, so re-enqueue it
        if (currentProcess && !processTerminated)
        {
            currentProcess->resetToHighestPriority();
            currentProcess->setState(ProcessState::READY);  // Set to READY when boosted
            currentProcess->setQueueEnterTime(currentTime);
            readyQueues[0].enqueue(currentProcess);
            currentProcess = nullptr;
        }
    }
    else
    {
        // Handle completion or quantum expiration
        if (processTerminated)
        {
            currentProcess->setCompletionTime(currentTime);
            currentProcess->calculateMetrics(currentTime);
            completedProcesses.push_back(currentProcess);
            currentProcess = nullptr;
        }
        else if (quantumExpired)
        {
            // Process used up its time quantum
            moveToNextQueue(currentProcess);
            currentProcess = nullptr;
        }
    }
}

bool MLFQScheduler::hasProcesses() const 
{
    // Check if there are processes in ready queues
    for (const auto& queue : readyQueues) 
    {
        if (!queue.isEmpty()) 
        {
            return true;
        }
    }
    
    // Check if current process is running
    if (currentProcess && currentProcess->getState() != ProcessState::TERMINATED) 
    {
        return true;
    }
    
    // Check if there are processes yet to arrive
    for (const auto& process : allProcesses) 
    {
        if (process->getArrivalTime() > currentTime) 
        {
            return true;
        }
    }
    
    return false;
}

bool MLFQScheduler::isComplete() const 
{
    return completedProcesses.size() == allProcesses.size();
}

SchedulerStats MLFQScheduler::getStats() const 
{
    SchedulerStats stats = {0, 0, 0, 0, 0, 0, 0};

    stats.totalProcesses = allProcesses.size();
    stats.completedProcesses = completedProcesses.size();
    stats.currentTime = currentTime;

    if (completedProcesses.empty()) 
    {
        return stats;
    }

    int totalWait = 0, totalTurnaround = 0, totalResponse = 0;

    for (const auto& process : completedProcesses) 
    {
        totalWait += process->getWaitTime();
        totalTurnaround += process->getTurnaroundTime();
        totalResponse += process->getResponseTime();
    }

    int count = completedProcesses.size();
    stats.avgWaitTime = static_cast<double>(totalWait) / count;
    stats.avgTurnaroundTime = static_cast<double>(totalTurnaround) / count;
    stats.avgResponseTime = static_cast<double>(totalResponse) / count;

    if (currentTime > 0)
    {
        // CPU Utilization = (actual CPU time used) / (total time) * 100
        // Only count time for processes that have actually executed

        int totalCpuTimeUsed = 0;
        for (const auto& process : completedProcesses)
        {
            totalCpuTimeUsed += process->getBurstTime();  // Completed processes used all their burst time
        }

        if (currentProcess && currentProcess->getState() != ProcessState::TERMINATED)
        {
            // Add time used by currently running process
            totalCpuTimeUsed += (currentProcess->getBurstTime() - currentProcess->getRemainingTime());
        }

        stats.cpuUtilization = (static_cast<double>(totalCpuTimeUsed) / currentTime) * 100.0;

        // Cap at 100% to avoid display issues
        if (stats.cpuUtilization > 100.0)
        {
            stats.cpuUtilization = 100.0;
        }
    }

    return stats;
}

void MLFQScheduler::reset() 
{
    currentTime = 0;
    boostTimer = 0;
    pidCounter = 1;  // Reset PID counter
    currentProcess = nullptr;
    completedProcesses.clear();
    executionLog.clear();
    allProcesses.clear();  // Clear all processes
    
    for (auto& queue : readyQueues) 
    {
        queue.clear();
    }
}

// Destructor
MLFQScheduler::~MLFQScheduler() 
{
    
}



