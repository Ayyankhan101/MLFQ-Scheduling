#include "MLFQScheduler.h"
#include "Visualizer.h"
#include <iostream>
#include <cassert>

void testProcessCreation() 
{
    std::cout << "Testing Process Creation... ";
    
    Process p1(1, 0, 10);
    assert(p1.getPid() == 1);
    assert(p1.getArrivalTime() == 0);
    assert(p1.getBurstTime() == 10);
    assert(p1.getRemainingTime() == 10);
    assert(p1.getState() == ProcessState::NEW);
    
    std::cout << "PASSED\n";
}

void testQueueOperations() 
{
    std::cout << "Testing Queue Operations... ";
    
    ProcessQueue queue(0, 4);
    assert(queue.isEmpty());
    
    auto p1 = std::make_shared<Process>(1, 0, 10);
    auto p2 = std::make_shared<Process>(2, 0, 20);
    
    queue.enqueue(p1);
    assert(!queue.isEmpty());
    assert(queue.size() == 1);
    
    queue.enqueue(p2);
    assert(queue.size() == 2);
    
    auto dequeued = queue.dequeue();
    assert(dequeued->getPid() == 1);
    assert(queue.size() == 1);
    
    std::cout << "PASSED\n";
}

void testSchedulerBasics() 
{
    std::cout << "Testing Scheduler Basics... ";
    
    MLFQScheduler scheduler(3, 50);
    
    scheduler.addProcess(0, 10);
    scheduler.addProcess(0, 5);
    
    assert(!scheduler.isComplete());
    
    // Run a few steps
    for (int i = 0; i < 5; i++) {
        scheduler.step();
    }
    
    assert(scheduler.getCurrentTime() == 5);
    
    std::cout << "PASSED\n";
}

void testSchedulerCompletion() 
{
    std::cout << "Testing Scheduler Completion... ";
    
    MLFQScheduler scheduler(3, 50);
    
    scheduler.addProcess(0, 3);
    scheduler.addProcess(0, 3);
    
    // Run until completion
    while (!scheduler.isComplete()) 
    {
        scheduler.step();
    }
    
    assert(scheduler.isComplete());
    assert(scheduler.getCompletedProcesses().size() == 2);
    
    auto stats = scheduler.getStats();
    assert(stats.completedProcesses == 2);
    assert(stats.totalProcesses == 2);
    
    std::cout << "PASSED\n";
}

void testMetricsCalculation() 
{
    std::cout << "Testing Metrics Calculation... ";
    
    MLFQScheduler scheduler(3, 100);
    
    // Simple test case: One process
    scheduler.addProcess(0, 10);
    
    while (!scheduler.isComplete()) 
    {
        scheduler.step();
    }
    
    auto stats = scheduler.getStats();
    assert(stats.avgWaitTime >= 0);
    assert(stats.avgTurnaroundTime > 0);
    assert(stats.avgResponseTime >= 0);
    assert(stats.cpuUtilization > 0);
    
    std::cout << "PASSED\n";
}

void runAllTests() {
    std::cout << "\n========================================\n";
    std::cout << "    MLFQ Scheduler Unit Tests\n";
    std::cout << "========================================\n\n";
    
    testProcessCreation();
    testQueueOperations();
    testSchedulerBasics();
    testSchedulerCompletion();
    testMetricsCalculation();
    
    std::cout << "\n========================================\n";
    std::cout << "    All Tests PASSED!\n";
    std::cout << "========================================\n\n";
}

int main() 
{
    runAllTests();
    return 0;
}
