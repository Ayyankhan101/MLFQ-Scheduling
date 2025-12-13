#ifndef SCHEDULER_CONFIG_H
#define SCHEDULER_CONFIG_H

#include <string>
#include <iostream>
using namespace std;

enum class LastQueueAlgorithm
{
    ROUND_ROBIN,
    SHORTEST_JOB_FIRST,
    PRIORITY_SCHEDULING
};

/**
  tunable parameters for the scheduling algorithm
**/
struct SchedulerConfig 
{
    // Core MLFQ Parameters
    int numQueues;              // Number of priority queues (2-5)
    int boostInterval;          // Priority boost interval in ms (20-500)
    int baseQuantum;            // Base time quantum for first queue (2-10)
    double quantumMultiplier;   // Growth factor for quantum (1.5, 2.0, 3.0)
    
    // Process Generation Parameters (for random/custom sets)
    int numProcesses;           // Number of processes to generate (1-20)
    int maxArrivalTime;         // Maximum arrival time window (0-100)
    int minBurstTime;           // Minimum burst time (1-50)
    int maxBurstTime;           // Maximum burst time (1-100)
    
    // Visualization Parameters
    int animationDelay;         // Delay between steps in ms (50-1000)
    
    // Default constructor with standard MLFQ values
    SchedulerConfig() 
        : numQueues(3),
          boostInterval(100),
          baseQuantum(4),
          quantumMultiplier(2.0),
          numProcesses(5),
          maxArrivalTime(20),
          minBurstTime(5),
          maxBurstTime(30),
          animationDelay(200) {}
    
    /**
     * Validate configuration parameters
     * Returns true if all parameters are within acceptable ranges
     */
    bool validate(string& errorMsg) const 
    {
        if (numQueues < 2 || numQueues > 5) 
        {
            errorMsg = "Number of queues must be between 2 and 5";
            return false;
        }
        if (boostInterval < 20 || boostInterval > 500) 
        {
            errorMsg = "Boost interval must be between 20 and 500 ms";
            return false;
        }
        if (baseQuantum < 2 || baseQuantum > 10) 
        {
            errorMsg = "Base quantum must be between 2 and 10 ms";
            return false;
        }
        if (quantumMultiplier < 1.0 || quantumMultiplier > 5.0) 
        {
            errorMsg = "Quantum multiplier must be between 1.0 and 5.0";
            return false;
        }
        if (numProcesses < 1 || numProcesses > 20) 
        {
            errorMsg = "Number of processes must be between 1 and 20";
            return false;
        }
        if (maxArrivalTime < 0 || maxArrivalTime > 100) 
        {
            errorMsg = "Max arrival time must be between 0 and 100 ms";
            return false;
        }
        if (minBurstTime < 1 || minBurstTime > 50) 
        {
            errorMsg = "Min burst time must be between 1 and 50 ms";
            return false;
        }
        if (maxBurstTime < minBurstTime || maxBurstTime > 100) 
        {
            errorMsg = "Max burst time must be between min burst time and 100 ms";
            return false;
        }
        if (animationDelay < 50 || animationDelay > 2000) 
        {
            errorMsg = "Animation delay must be between 50 and 2000 ms";
            return false;
        }
        return true;
    }
    
    /**
     * Display current configuration
     */
    void display() const 
    {
        cout << "\n╔═══════════════════════════════════════════════════════╗\n";
        cout <<   "║           CURRENT MLFQ CONFIGURATION                  ║\n";
        cout <<   "╠═══════════════════════════════════════════════════════╣\n";
        cout <<   "║ Core Algorithm Parameters:                            ║\n";
        cout <<   "║   • Number of Queues:    " << numQueues << "          ║\n";                
        cout <<   "║   • Boost Interval:      " << boostInterval << " ms   ║\n";                   
        cout <<   "║   • Base Time Quantum:   " << baseQuantum << " ms     ║\n";                  
        cout <<   "║   • Quantum Multiplier:  " << quantumMultiplier << "x ║\n";
        cout <<   "╠═══════════════════════════════════════════════════════╣\n";
        cout <<   "║ Process Generation:                                   ║\n";
        cout <<   "║   • Number of Processes:" << numProcesses << "        ║\n";
        cout <<   "║   • Max Arrival Time:" << maxArrivalTime << " ms      ║\n";
        cout <<   "║   • Burst Time Range:" << minBurstTime << "-" << maxBurstTime << " ms  ║\n";
        cout <<   "╠═══════════════════════════════════════════════════════╣\n";
        cout <<   "║ Visualization:                                        ║\n";
        cout <<   "║   • Animation Delay:     " << animationDelay << " ms  ║\n";
        cout <<   "╚═══════════════════════════════════════════════════════╝\n";
    }
    
    /**
     * Get quantum for a specific queue level
     */
    int getQuantumForQueue(int queueLevel) const 
    {
        int quantum = baseQuantum;
        for (int i = 0; i < queueLevel; i++) 
        {
            quantum = static_cast<int>(quantum * quantumMultiplier);
        }
        return quantum;
    }
    
    /**
     * Display quantum values for all queues
     */
    void displayQuantums() const 
    {
        cout << "  Time Quantums: ";
        for (int i = 0; i < numQueues; i++) 
        {
            cout << "Q" << i << "=" << getQuantumForQueue(i) << "ms";
            if (i < numQueues - 1) cout << ", ";
        }
        cout << "\n";
    }
};

/**
 * Helper class for interactive configuration
 */
class ConfigurationManager 
{
public:
    /**
     * Interactive terminal-based configuration
     */
    static SchedulerConfig configureFromTerminal() 
    {
        SchedulerConfig config;
        
        cout << "\n╔═══════════════════════════════════════════════════════╗\n";
        cout <<   "║         MLFQ SCHEDULER CONFIGURATION                  ║\n";
        cout <<   "╠═══════════════════════════════════════════════════════╣\n";
        cout <<   "║ Configure algorithm parameters                        ║\n";
        cout <<   "║ (Press Enter to use default values shown in [])       ║\n";
        cout <<   "╚═══════════════════════════════════════════════════════╝\n\n";
        
        // Core parameters
        config.numQueues = getIntInput
        (
            "Number of queues [2-5]", 
            config.numQueues, 2, 5
        );
        
        config.baseQuantum = getIntInput
        (
            "Base time quantum (ms) [2-10]", 
            config.baseQuantum, 2, 10
        );
        
        config.quantumMultiplier = getDoubleInput
        (
            "Quantum growth factor [1.5-5.0]", 
            config.quantumMultiplier, 1.0, 5.0
        );
        
        config.boostInterval = getIntInput
        (
            "Priority boost interval (ms) [20-500]", 
            config.boostInterval, 20, 500
        );
        
        cout << "\n--- Process Generation Parameters ---\n";
        
        config.numProcesses = getIntInput
        (
            "Number of processes [1-20]", 
            config.numProcesses, 1, 20
        );
        
        config.maxArrivalTime = getIntInput
        (
            "Max arrival time (ms) [0-100]", 
            config.maxArrivalTime, 0, 100
        );
        
        config.minBurstTime = getIntInput
        (
            "Min burst time (ms) [1-50]", 
            config.minBurstTime, 1, 50
        );
        
        config.maxBurstTime = getIntInput
        (
            "Max burst time (ms) [" + to_string(config.minBurstTime) + "-100]", 
            config.maxBurstTime, config.minBurstTime, 100
        );
        
        cout << "\n--- Visualization Parameters ---\n";
        
        config.animationDelay = getIntInput
        (
            "Animation delay (ms) [50-2000]", 
            config.animationDelay, 50, 2000
        );
        
        // Validate and display
        string errorMsg;
        if (!config.validate(errorMsg)) 
        {
            cout << "\nConfiguration Error: " << errorMsg << "\n";
            cout << "Using default configuration instead.\n";
            return SchedulerConfig();
        }
        
        cout << "\nConfiguration complete!\n";
        config.display();
        config.displayQuantums();
        
        return config;
    }
    
    /**
     * Quick preset configurations
     */
    static SchedulerConfig getPreset(int presetNumber) 
    {
        SchedulerConfig config;
        
        switch (presetNumber) 
        {
            case 1: // Standard MLFQ
                config.numQueues = 3;
                config.baseQuantum = 4;
                config.quantumMultiplier = 2.0;
                config.boostInterval = 100;
                break;
                
            case 2: // Fine-grained (more queues, smaller quantums)
                config.numQueues = 5;
                config.baseQuantum = 2;
                config.quantumMultiplier = 1.5;
                config.boostInterval = 50;
                break;
                
            case 3: // Coarse-grained (fewer queues, larger quantums)
                config.numQueues = 2;
                config.baseQuantum = 8;
                config.quantumMultiplier = 3.0;
                config.boostInterval = 200;
                break;
                
            case 4: // Aggressive boost (prevent starvation aggressively)
                config.numQueues = 3;
                config.baseQuantum = 4;
                config.quantumMultiplier = 2.0;
                config.boostInterval = 30;
                break;
                
            default:
                break; // Use default values
        }
        
        return config;
    }
    
private:
    static int getIntInput(const string& prompt, int defaultValue, int min, int max) 
    {
        cout << prompt << " [" << defaultValue << "]: ";
        string input;
        getline(cin, input);
        
        if (input.empty()) 
        {
            return defaultValue;
        }
        
        try 
        {
            int value = stoi(input);

            if (value < min || value > max) 
            {
                cout << "  ⚠ Value out of range. Using default: " << defaultValue << "\n";
                return defaultValue;
            }
            return value;
        } 
        catch (...) 
        {
            cout << "  ⚠ Invalid input. Using default: " << defaultValue << "\n";
            return defaultValue;
        }
    }
    
    static double getDoubleInput(const string& prompt, double defaultValue, double min, double max) {
        cout << prompt << " [" << defaultValue << "]: ";
        string input;
        getline(cin, input);
        
        if (input.empty()) 
        {
            return defaultValue;
        }
        
        try 
        {
            double value = stod(input);
            if (value < min || value > max) 
            {
                cout << "  ⚠ Value out of range. Using default: " << defaultValue << "\n";
                return defaultValue;
            }
            return value;
        } 
        catch (...) 
        {
            cout << "  ⚠ Invalid input. Using default: " << defaultValue << "\n";
            return defaultValue;
        }
    }
};

#endif // SCHEDULER_CONFIG_H
