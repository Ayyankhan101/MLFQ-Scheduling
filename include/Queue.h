#ifndef QUEUE_H
#define QUEUE_H

#include "Process.h"
#include <deque>
#include <memory>
using namespace std;

class ProcessQueue 
{
private:
    deque<shared_ptr<Process>> queue;
    int queueLevel;
    int timeQuantum;

public:
    ProcessQueue(int level, int quantum);
    
    // Queue operations
    void enqueue(shared_ptr<Process> process);
    shared_ptr<Process> dequeue();
    shared_ptr<Process> peek() const;
    
    // Getters
    bool isEmpty() const { return queue.empty(); }
    size_t size() const { return queue.size(); }
    int getQueueLevel() const { return queueLevel; }
    int getTimeQuantum() const { return timeQuantum; }
    
    // Get all processes (for visualization)
    const deque<shared_ptr<Process>>& getProcesses() const { return queue; }
    
    // Remove specific process
    bool removeProcess(int pid);
    
    // Clear queue
    void clear() { queue.clear(); }
};

#endif // QUEUE_H
