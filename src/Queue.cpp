#include "Queue.h"
#include <algorithm>
using namespace std;

ProcessQueue::ProcessQueue(int level, int quantum)
    : queueLevel(level), timeQuantum(quantum) {}

void ProcessQueue::enqueue(shared_ptr<Process> process) 
{
    if (process) 
    {
        process->setPriority(queueLevel);
        process->setState(ProcessState::READY);
        queue.push_back(process);
    }
}

shared_ptr<Process> ProcessQueue::dequeue() 
{
    if (queue.empty()) 
    {
        return nullptr;
    }
    
    auto process = queue.front();
    queue.pop_front();
    return process;
}

shared_ptr<Process> ProcessQueue::peek() const 
{
    if (queue.empty()) 
    {
        return nullptr;
    }
    return queue.front();
}

bool ProcessQueue::removeProcess(int pid) 
{
    auto it = find_if(queue.begin(), queue.end(),
        [pid](const shared_ptr<Process>& p) 
        {
            return p->getPid() == pid;
        });
    
    if (it != queue.end()) 
    {
        queue.erase(it);
        return true;
    }
    return false;
}
