#include "MLFQScheduler.h"
#include "Visualizer.h"
#include "SchedulerConfig.h"
#include "TerminalUI.h"
#include "WebServer.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <memory>  // For smart pointers
#include <random>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <limits>
#include <thread>
#include <chrono>

#ifdef FLTK_AVAILABLE
#include "FLTKVisualizer.h"
#endif

using namespace std;
using namespace TerminalUI;

// Global variable to store the selected example set across all menu options
static int globalExampleSet = 0;  // 0 means no example set selected

// Function declarations  
void runInteractiveMode(MLFQScheduler& scheduler, Visualizer& viz);
void runAutoMode(MLFQScheduler& scheduler, Visualizer& viz);
void runGUIMode(MLFQScheduler& scheduler);
void runWebGUIMode(MLFQScheduler& scheduler);
void runAlgorithmSwitchMode(MLFQScheduler& scheduler, Visualizer& viz);
void saveResultsToCSV(MLFQScheduler& scheduler);
void displayMenu();
void loadExampleProcessSet(MLFQScheduler& scheduler);
void loadExampleProcessSet(MLFQScheduler& scheduler, int setNumber);
void displayPresetMenu();
void compareLastQueueAlgorithms();
void createCustomProcesses(MLFQScheduler& scheduler);
void generateRandomProcesses(MLFQScheduler& scheduler);

void runInteractiveMode(MLFQScheduler& scheduler, Visualizer& viz)
{
    cout << "\n" << TerminalUI::Style::header("=== MLFQ Scheduler - Interactive Mode ===") << "\n";
    scheduler.getConfig().displayQuantums();

    // Show initial process information
    cout << "\n" << TerminalUI::Style::highlight("Initial Process Set:") << "\n";
    viz.displayProcessTable();

    cout << "\nPress Enter to start execution...\n";
    cin.get();

    while (!scheduler.isComplete())
    {
        cout << "\n[Press Enter to continue, 'a' for auto mode, 'q' to quit]: ";
        string input;
        getline(cin, input);

        if (input == "q" || input == "Q")
        {
            break;
        }
        else if (input == "a" || input == "A")
        {
            // Auto mode
            while (!scheduler.isComplete())
            {
                scheduler.step();
                viz.displayAllWithProcessInfo();
                this_thread::sleep_for(chrono::milliseconds(100));
            }
            break;
        }

        scheduler.step();
        viz.displayAllWithProcessInfo();
    }

    viz.displayAll();
    cout << "\n" << TerminalUI::Style::success("=== Scheduling Complete ===") << "\n";
}

void runAutoMode(MLFQScheduler& scheduler, Visualizer& viz)
{
    cout << "\n" << TerminalUI::Style::header("=== MLFQ Scheduler - Auto Mode ===") << "\n";
    scheduler.getConfig().displayQuantums();

    // Show initial process information
    cout << "\n" << TerminalUI::Style::highlight("Initial Process Set:") << "\n";
    viz.displayProcessTable();

    cout << "\nPress Enter to start automatic execution...\n";
    cin.get();

    int delay = scheduler.getConfig().animationDelay;
    while (!scheduler.isComplete())
    {
        viz.displayAllWithProcessInfo();
        scheduler.step();
        this_thread::sleep_for(chrono::milliseconds(delay));
    }

    viz.displayAll();
    cout << "\n" << TerminalUI::Style::success("=== Scheduling Complete ===") << "\n";

    // Offer to save results
    cout << "\n" << TerminalUI::Style::info("Save results to file? (y/n): ");
    string saveChoice;
    try
    {
        getline(cin, saveChoice);
        if (saveChoice == "y" || saveChoice == "Y")
        {
            saveResultsToCSV(scheduler);
        }
    }
    catch (const exception& e)
    {
        cout << TerminalUI::Style::error("Error during file save operation: " + string(e.what())) << "\n";
    }
}

void runQuickMode(MLFQScheduler& scheduler, Visualizer& viz)
{
    cout << "\n" << TerminalUI::Style::warning("=== Quick Run Mode ===") << "\n";
    cout << "Running simulation without animation...\n\n";

    auto startTime = chrono::high_resolution_clock::now();

    while (!scheduler.isComplete())
    {
        scheduler.step();
    }

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);

    // Show results
    TerminalUI::clearScreen();
    viz.displayProcessTable();
    viz.displayStats();
    viz.displayGanttChart();

    cout << "\n" << TerminalUI::Style::success("Execution Time: " + to_string(duration.count()) + " ms (real time)") << "\n";
    cout << "\n" << TerminalUI::Style::success("=== Quick Run Complete ===") << "\n";

    cout << "\nPress Enter to continue...";
    cin.get();
}

void saveResultsToCSV(MLFQScheduler& scheduler)
{
    string filename = "mlfq_results_" + to_string(time(nullptr)) + ".csv";

    try
    {
        ofstream file(filename);

        if (!file.is_open())
        {
            cout << TerminalUI::Style::error("Error: Could not create file!") << "\n";
            return;
        }

        // Write header
        file << "PID,Arrival,Burst,Completion,Turnaround,Wait,Response\n";

        // Write process data
        const auto& completed = scheduler.getCompletedProcesses();
        for (const auto& proc : completed)
        {
            file << proc->getPid() << ","
                 << proc->getArrivalTime() << ","
                 << proc->getBurstTime() << ","
                 << proc->getCompletionTime() << ","
                 << proc->getTurnaroundTime() << ","
                 << proc->getWaitTime() << ","
                 << proc->getResponseTime() << "\n";
        }

        // summary statistics
        auto stats = scheduler.getStats();
        file << "\nSummary Statistics\n";
        file << "Total Processes," << stats.totalProcesses << "\n";
        file << "Completed," << stats.completedProcesses << "\n";
        file << "Total Time," << stats.currentTime << "\n";
        file << "Avg Wait Time," << fixed << setprecision(2) << stats.avgWaitTime << "\n";
        file << "Avg Turnaround Time," << stats.avgTurnaroundTime << "\n";
        file << "Avg Response Time," << stats.avgResponseTime << "\n";
        file << "CPU Utilization," << stats.cpuUtilization << "%\n";

        file.close();
        cout << TerminalUI::Style::success("✓ Results saved to: " + filename) << "\n";
    }
    catch (const exception& e)
    {
        cout << TerminalUI::Style::error("Error saving file: " + string(e.what())) << "\n";
    }
}

void runComparisonMode()
{
    cout << "\n" << TerminalUI::Style::warning("=== Algorithm Comparison Mode ===") << "\n";
    cout << "Compare Round Robin, SJF, and Priority Scheduling\n\n";

    // Create processes
    vector<pair<int, int>> processes =
    {
        {0, 20}, {5, 12}, {10, 8}, {15, 16}, {20, 5}
    };

    cout << "Test processes: ";
    for (size_t i = 0; i < processes.size(); i++)
    {
        cout << "P" << (i+1) << "(" << processes[i].first << "," << processes[i].second << ") ";
    }
    cout << "\n\n";

    struct AlgoResult
    {
        string name;
        double avgWait;
        double avgTurnaround;
        double avgResponse;
        double cpuUtil;
        int totalTime;
    };

    vector<AlgoResult> results;

    // Test each algorithm
    LastQueueAlgorithm algos[] =
    {
        LastQueueAlgorithm::ROUND_ROBIN,
        LastQueueAlgorithm::SHORTEST_JOB_FIRST,
        LastQueueAlgorithm::PRIORITY_SCHEDULING
    };

    string algoNames[] = {"Round Robin", "Shortest Job First", "Priority Scheduling"};

    for (int i = 0; i < 3; i++) 
    {
        cout << "Testing " << algoNames[i] << "...\n";

        MLFQScheduler scheduler(3, 100);
        scheduler.setLastQueueAlgorithm(algos[i]);

        for (const auto& p : processes)
        {
            scheduler.addProcess(p.first, p.second);
        }

        while (!scheduler.isComplete())
        {
            scheduler.step();
        }

        auto stats = scheduler.getStats();
        results.push_back
        ({
            algoNames[i],
            stats.avgWaitTime,
            stats.avgTurnaroundTime,
            stats.avgResponseTime,
            stats.cpuUtilization,
            stats.currentTime
        });
    }

    // Display comparison table
    cout << "\n" << TerminalUI::Style::info("═══ COMPARISON RESULTS ═══") << "\n\n";
    cout << left << setw(20) << "Algorithm"
              << right << setw(12) << "Avg Wait"
              << setw(12) << "Avg TAT"
              << setw(12) << "Avg Resp"
              << setw(10) << "Time"
              << setw(10) << "CPU%" << "\n";
    cout << string(76, '-') << "\n";

    for (const auto& r : results)
    {
        cout << left << setw(20) << r.name
                  << right << fixed << setprecision(2)
                  << setw(12) << r.avgWait
                  << setw(12) << r.avgTurnaround
                  << setw(12) << r.avgResponse
                  << setw(10) << r.totalTime
                  << setw(9) << r.cpuUtil << "%" << "\n";
    }

    // Find best algorithm
    cout << "\n" << TerminalUI::Style::success("Best Performance:") << "\n";
    auto minWait = min_element(results.begin(), results.end(),
        [](const auto& a, const auto& b) { return a.avgWait < b.avgWait; });
    cout << "  Lowest Avg Wait: " << minWait->name << " (" << minWait->avgWait << " ms)\n";

    auto minTAT = min_element(results.begin(), results.end(),
        [](const auto& a, const auto& b) { return a.avgTurnaround < b.avgTurnaround; });
    cout << "  Lowest Avg TAT:  " << minTAT->name << " (" << minTAT->avgTurnaround << " ms)\n";

    cout << "\nPress Enter to continue...";
    cin.get();
}

void runGUIMode(MLFQScheduler& scheduler) 
{
    cout << "\n=== MLFQ Scheduler - GUI Mode ===\n";
    #ifdef FLTK_AVAILABLE
    cout << "Starting FLTK GUI visualization...\n";
    FLTKVisualizer gui(scheduler);
    gui.run();
    #else
    (void)scheduler; // Suppress unused parameter warning
    cout << "FLTK not available. Please install FLTK libraries to use GUI mode.\n";
    cout << "Running in terminal mode instead...\n";
    #endif
}

void runAlgorithmSwitchMode(MLFQScheduler& scheduler, Visualizer& viz) 
{
    TerminalUI::drawHeader("ALGORITHM-SWITCHING MLFQ SCHEDULER", 80);
    cout << " Dynamic last-queue algorithm selection " << endl;
    TerminalUI::drawSeparator(80);

    // Show algorithm selection menu
    cout << "\nSelect algorithm for last queue:\n";
    cout << "1. Round Robin (RR)\n";
    cout << "2. Shortest Job First (SJF)\n";
    cout << "3. Priority Scheduling\n";
    cout << "Enter choice (1-3): ";

    int choice;
    cin >> choice;
    cin.ignore(); // Clear newline

    LastQueueAlgorithm algorithm;
    switch(choice)
    {
        case 1:
            algorithm = LastQueueAlgorithm::ROUND_ROBIN;
            cout << "Selected: Round Robin for last queue\n";
            break;
        case 2:
            algorithm = LastQueueAlgorithm::SHORTEST_JOB_FIRST;
            cout << "Selected: Shortest Job First for last queue\n";
            break;
        case 3:
            algorithm = LastQueueAlgorithm::PRIORITY_SCHEDULING;
            cout << "Selected: Priority Scheduling for last queue\n";
            break;
        default:
            algorithm = LastQueueAlgorithm::ROUND_ROBIN;
            cout << "Invalid choice. Defaulting to Round Robin for last queue\n";
            break;
    }

    scheduler.setLastQueueAlgorithm(algorithm);
    scheduler.getConfig().displayQuantums();
    cout << "Current last queue algorithm: ";

    switch(algorithm)
    {
        case LastQueueAlgorithm::ROUND_ROBIN:
            cout << "Round Robin\n";
            break;
        case LastQueueAlgorithm::SHORTEST_JOB_FIRST:
            cout << "Shortest Job First\n";
            break;
        case LastQueueAlgorithm::PRIORITY_SCHEDULING:
            cout << "Priority Scheduling\n";
            break;
    }

    cout << "\nExecution Mode:\n";
    cout << "1. Interactive (step-by-step)\n";
    cout << "2. Automatic (continuous)\n";
    cout << "Enter mode: ";

    int mode;
    cin >> mode;
    cin.ignore();

    if (mode == 1)
    {
        runInteractiveMode(scheduler, viz);
    }
     else if (mode == 2)
     {
        runAutoMode(scheduler, viz);
    }
     else
     {
        cout << "Invalid mode! Defaulting to Auto mode...\n";
        runAutoMode(scheduler, viz);
    }
}

void displayMenu()
{
    TerminalUI::clearScreen();
    TerminalUI::drawHeader("MLFQ SCHEDULER", 80);
    cout << " " << TerminalUI::Style::info("Multilevel Feedback Queue Scheduling") << endl;
    TerminalUI::drawSeparator(80);

    cout << endl;
    cout << " " << TerminalUI::Style::success("Process Sets") << endl;
    cout << "   " << TerminalUI::Style::highlight("1") << ". Run with example process set" << endl;
    cout << "   " << TerminalUI::Style::highlight("2") << ". Create custom processes" << endl;
    cout << "   " << TerminalUI::Style::highlight("3") << ". Generate random processes" << endl;

    #ifdef FLTK_AVAILABLE
    cout << endl;
    cout << " " << TerminalUI::Style::info("Visualization") << endl;
    cout << "   " << TerminalUI::Style::highlight("4") << ". Run with FLTK GUI visualization" << endl;
    cout << "   " << TerminalUI::Style::highlight("5") << ". Run with Web GUI (Browser)" << endl;

    cout << endl;
    cout << " " << TerminalUI::Style::warning("Advanced") << endl;
    cout << "   " << TerminalUI::Style::highlight("6") << ". Run with algorithm-switching scheduler" << endl;
    cout << "   " << TerminalUI::Style::highlight("7") << ". Configure scheduler parameters" << endl;
    cout << "   " << TerminalUI::Style::highlight("8") << ". Load preset configuration" << endl;
    cout << "   " << TerminalUI::Style::highlight("9") << ". Load example process set with default values" << endl;
    cout << "   " << TerminalUI::Style::highlight("10") << ". Compare Last Queue Algorithms" << endl;

    cout << endl;
    cout << " " << TerminalUI::Style::error("Exit") << endl;
    cout << "    " << TerminalUI::Style::highlight("11") << ". Exit program" << endl;
    #else
    cout << endl;
    cout << " " << TerminalUI::Style::warning("Advanced") << endl;
    cout << "   " << TerminalUI::Style::highlight("4") << ". Run with algorithm-switching scheduler" << endl;
    cout << "   " << TerminalUI::Style::highlight("5") << ". Configure scheduler parameters" << endl;
    cout << "   " << TerminalUI::Style::highlight("6") << ". Load preset configuration" << endl;
    cout << "   " << TerminalUI::Style::highlight("7") << ". Load example process set with default values" << endl;
    cout << "   " << TerminalUI::Style::highlight("8") << ". Compare Last Queue Algorithms" << endl;

    cout << endl;
    cout << " " << TerminalUI::Style::error("Exit") << endl;
    cout << "    " << TerminalUI::Style::highlight("9") << ". Exit program" << endl;
    #endif

    cout << endl << " " << TerminalUI::Style::success("▶") << " Enter choice: ";
}

void loadExampleProcessSet(MLFQScheduler& scheduler)
{
    // Example process set demonstrating MLFQ behavior
    scheduler.addProcess(0, 20);   // P1: arrives at 0, burst 20
    scheduler.addProcess(5, 12);   // P2: arrives at 5, burst 12
    scheduler.addProcess(10, 8);   // P3: arrives at 10, burst 8
    scheduler.addProcess(15, 16);  // P4: arrives at 15, burst 16
    scheduler.addProcess(20, 5);   // P5: arrives at 20, burst 5
}

void loadExampleProcessSet(MLFQScheduler& scheduler, int setNumber)
{
    switch(setNumber) {
        case 1: // Standard Set: 5 processes with varying arrival
            scheduler.addProcess(0, 20);
            scheduler.addProcess(5, 12);
            scheduler.addProcess(10, 8);
            scheduler.addProcess(15, 16);
            scheduler.addProcess(20, 5);
            break;
        case 2: // CPU-Intensive: 3 long-running processes
            scheduler.addProcess(0, 30);
            scheduler.addProcess(5, 25);
            scheduler.addProcess(10, 20);
            break;
        case 3: // I/O-Intensive: 5 short processes
            scheduler.addProcess(0, 3);
            scheduler.addProcess(2, 2);
            scheduler.addProcess(4, 4);
            scheduler.addProcess(6, 3);
            scheduler.addProcess(8, 2);
            break;
        case 4: // Default Set: Same as Standard Set (fallback)
            scheduler.addProcess(0, 20);
            scheduler.addProcess(5, 12);
            scheduler.addProcess(10, 8);
            scheduler.addProcess(15, 16);
            scheduler.addProcess(20, 5);
            break;
        default:
            loadExampleProcessSet(scheduler);
    }
}

void createCustomProcesses(MLFQScheduler& scheduler)
{
    cout << "\n" << TerminalUI::Style::header("=== Create Custom Processes ===") << "\n";
    
    int numProcesses;
    cout << "Number of processes (1-10): ";
    while(!(cin >> numProcesses) || numProcesses < 1 || numProcesses > 10) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid input! Enter number between 1-10: ";
    }
    
    for(int i = 0; i < numProcesses; i++) {
        int arrival, burst;
        cout << "Process " << (i+1) << " - Arrival time (0-50): ";
        while(!(cin >> arrival) || arrival < 0 || arrival > 50) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid! Enter arrival time (0-50): ";
        }
        cout << "Process " << (i+1) << " - Burst time (1-30): ";
        while(!(cin >> burst) || burst < 1 || burst > 30) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid! Enter burst time (1-30): ";
        }
        scheduler.addProcess(arrival, burst);
    }
    cin.ignore();
    cout << "\n" << numProcesses << " processes added successfully!\n";
}

void generateRandomProcesses(MLFQScheduler& scheduler)
{
    cout << "\n" << TerminalUI::Style::header("=== Generate Random Processes ===") << "\n";
    
    int numProc, maxArr, minBurst, maxBurst;
    cout << "Number of processes (3-10): ";
    while(!(cin >> numProc) || numProc < 3 || numProc > 10) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid! Enter number between 3-10: ";
    }
    cout << "Max arrival time (5-20): ";
    while(!(cin >> maxArr) || maxArr < 5 || maxArr > 20) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid! Enter max arrival (5-20): ";
    }
    cout << "Min burst time (2-5): ";
    while(!(cin >> minBurst) || minBurst < 2 || minBurst > 5) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid! Enter min burst (2-5): ";
    }
    cout << "Max burst time (8-25): ";
    while(!(cin >> maxBurst) || maxBurst < 8 || maxBurst > 25 || maxBurst <= minBurst) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid! Enter max burst (8-25, > min burst): ";
    }
    cin.ignore();
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> arrivalDist(0, maxArr);
    uniform_int_distribution<> burstDist(minBurst, maxBurst);
    
    cout << "\nGenerated processes:\n";
    for(int i = 0; i < numProc; i++) {
        int arrival = arrivalDist(gen);
        int burst = burstDist(gen);
        scheduler.addProcess(arrival, burst);
        cout << "P" << (i+1) << ": Arrival=" << arrival << ", Burst=" << burst << "\n";
    }
    cout << "\n" << numProc << " random processes generated!\n";
}

void displayExampleSetsMenu()
{
    TerminalUI::drawHeader("EXAMPLE PROCESS SETS", 80);

    cout << " 1. Standard Set: 5 processes with varying arrival" << endl;
    cout << "    • P1: Arr=0, Burst=20" << endl;
    cout << "    • P2: Arr=5, Burst=12" << endl;
    cout << "    • P3: Arr=10, Burst=8" << endl;
    cout << "    • P4: Arr=15, Burst=16" << endl;
    cout << "    • P5: Arr=20, Burst=5" << endl;
    cout << endl;
    cout << " 2. CPU-Intensive: 3 long-running processes" << endl;
    cout << "    • P1: Arr=0, Burst=30" << endl;
    cout << "    • P2: Arr=5, Burst=25" << endl;
    cout << "    • P3: Arr=10, Burst=20" << endl;
    cout << endl;
    cout << " 3. I/O-Intensive: 5 short processes" << endl;
    cout << "    • P1: Arr=0, Burst=3" << endl;
    cout << "    • P2: Arr=2, Burst=2" << endl;
    cout << "    • P3: Arr=4, Burst=4" << endl;
    cout << "    • P4: Arr=6, Burst=3" << endl;
    cout << "    • P5: Arr=8, Burst=2" << endl;
    cout << endl;
    cout << " 4. Default Set: Same as Standard Set (fallback)" << endl;
    cout << endl;
    cout << " 5. Skip - Use currently configured values" << endl;

    cout << endl << "Select example set [1-5]: ";
}

void displayPresetMenu()
{
    TerminalUI::drawHeader("CONFIGURATION PRESETS", 80);

    cout << " 1. Standard MLFQ (3 queues, moderate boost)" << endl;
    cout << "    • Queues: 3 | Quantum: 4ms×2 | Boost: 100ms" << endl;
    cout << endl;
    cout << " 2. Fine-Grained (5 queues, frequent boost)" << endl;
    cout << "    • Queues: 5 | Quantum: 2ms×1.5 | Boost: 50ms" << endl;
    cout << endl;
    cout << " 3. Coarse-Grained (2 queues, rare boost)" << endl;
    cout << "    • Queues: 2 | Quantum: 8ms×3 | Boost: 200ms" << endl;
    cout << endl;
    cout << " 4. Aggressive Anti-Starvation (frequent boost)" << endl;
    cout << "    • Queues: 3 | Quantum: 4ms×2 | Boost: 30ms" << endl;
    cout << endl;
    cout << " 5. Keep current configuration" << endl;

    cout << endl << "Select preset [1-5]: ";
}

void compareLastQueueAlgorithms()
{
    cout << "\n" << TerminalUI::Style::header("=== Last Queue Algorithm Comparison ===") << "\n";
    
    cout << "Select process set for comparison:\n";
    cout << "1. Default example set (5 processes)\n";
    cout << "2. Choose from example sets\n";
    cout << "3. Generate random processes\n";
    cout << "4. Create custom processes\n";
    cout << "Enter choice (1-4): ";
    
    int choice;
    while(!(cin >> choice) || choice < 1 || choice > 4) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid! Enter choice (1-4): ";
    }
    cin.ignore();
    
    vector<pair<int, int>> processes;
    
    switch(choice) {
        case 1:
            processes = {{0, 20}, {5, 12}, {10, 8}, {15, 16}, {20, 5}};
            break;
        case 2:
            displayExampleSetsMenu();
            int exampleChoice;
            while(!(cin >> exampleChoice) || exampleChoice < 1 || exampleChoice > 5) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid! Enter choice (1-5): ";
            }
            cin.ignore();
            // Create temporary scheduler to load example set
            {
                MLFQScheduler tempScheduler(3, 100);
                loadExampleProcessSet(tempScheduler, exampleChoice);
                auto tempProcesses = tempScheduler.getAllProcesses();
                for(const auto& p : tempProcesses) {
                    processes.push_back({p->getArrivalTime(), p->getBurstTime()});
                }
            }
            break;
        case 3:
            cout << "Number of processes (3-10): ";
            int numProc;
            cin >> numProc;
            cout << "Max arrival time (0-20): ";
            int maxArr;
            cin >> maxArr;
            cout << "Min burst time (2-10): ";
            int minBurst;
            cin >> minBurst;
            cout << "Max burst time (5-25): ";
            int maxBurst;
            cin >> maxBurst;
            cin.ignore();
            
            {
                random_device rd;
                mt19937 gen(rd());
                uniform_int_distribution<> arrivalDist(0, maxArr);
                uniform_int_distribution<> burstDist(minBurst, maxBurst);
                
                for(int i = 0; i < numProc; i++) {
                    processes.push_back({arrivalDist(gen), burstDist(gen)});
                }
            }
            break;
        case 4:
            cout << "Number of processes: ";
            cin >> numProc;
            cin.ignore();
            for(int i = 0; i < numProc; i++) {
                int arr, burst;
                cout << "Process " << (i+1) << " - Arrival time: ";
                cin >> arr;
                cout << "Process " << (i+1) << " - Burst time: ";
                cin >> burst;
                processes.push_back({arr, burst});
            }
            cin.ignore();
            break;
        default:
            processes = {{0, 20}, {5, 12}, {10, 8}, {15, 16}, {20, 5}};
    }
    
    cout << "\nUsing " << processes.size() << " processes for comparison...\n";
    
    struct Result {
        string name;
        double avgWait;
        double avgTurnaround;
        double avgResponse;
        double cpuUtil;
        int totalTime;
    };
    
    vector<Result> results;
    
    LastQueueAlgorithm algos[] = {
        LastQueueAlgorithm::ROUND_ROBIN,
        LastQueueAlgorithm::SHORTEST_JOB_FIRST,
        LastQueueAlgorithm::PRIORITY_SCHEDULING
    };
    
    string algoNames[] = {"Round Robin", "Shortest Job First", "Priority Scheduling"};
    
    for (int i = 0; i < 3; i++) {
        cout << "Testing " << algoNames[i] << "...\n";
        
        MLFQScheduler scheduler(3, 100);
        scheduler.setLastQueueAlgorithm(algos[i]);
        
        for (const auto& p : processes) {
            scheduler.addProcess(p.first, p.second);
        }
        
        while (!scheduler.isComplete()) {
            scheduler.step();
        }
        
        auto stats = scheduler.getStats();
        results.push_back({
            algoNames[i],
            stats.avgWaitTime,
            stats.avgTurnaroundTime,
            stats.avgResponseTime,
            stats.cpuUtilization,
            stats.currentTime
        });
    }
    
    // Display comparison table
    cout << "\n" << TerminalUI::Style::info("═══ COMPARISON RESULTS ═══") << "\n\n";
    cout << left << setw(20) << "Algorithm"
         << right << setw(12) << "Avg Wait"
         << setw(12) << "Avg TAT"
         << setw(12) << "Avg Resp"
         << setw(10) << "Time"
         << setw(10) << "CPU%" << "\n";
    cout << string(76, '-') << "\n";
    
    for (const auto& r : results) {
        cout << left << setw(20) << r.name
             << right << fixed << setprecision(2)
             << setw(12) << r.avgWait
             << setw(12) << r.avgTurnaround
             << setw(12) << r.avgResponse
             << setw(10) << r.totalTime
             << setw(9) << r.cpuUtil << "%" << "\n";
    }
    
    // Find best algorithm
    cout << "\n" << TerminalUI::Style::success("Best Performance:") << "\n";
    auto minWait = min_element(results.begin(), results.end(),
        [](const auto& a, const auto& b) { return a.avgWait < b.avgWait; });
    cout << "  Lowest Avg Wait: " << minWait->name << " (" << minWait->avgWait << " ms)\n";
    
    auto minTAT = min_element(results.begin(), results.end(),
        [](const auto& a, const auto& b) { return a.avgTurnaround < b.avgTurnaround; });
    cout << "  Lowest Avg TAT:  " << minTAT->name << " (" << minTAT->avgTurnaround << " ms)\n";
    
    cout << "\nPress Enter to continue...";
    cin.get();
}

int main()
{
    SchedulerConfig config;  // Default configuration
    unique_ptr<MLFQScheduler> scheduler = make_unique<MLFQScheduler>(config);
    unique_ptr<Visualizer> viz = make_unique<Visualizer>(*scheduler);

    while (true)
    {
        displayMenu();

        int choice;
        // Validate menu input to prevent crashes
        while (!(cin >> choice))
        {
            cin.clear(); // Clear error flag
            cin.ignore(10000, '\n'); // Ignore invalid input
            cout << "\n" << TerminalUI::Style::error("Invalid input! Please enter a number: ");
        }
        cin.ignore(); // Clear newline

        #ifdef FLTK_AVAILABLE
        if (choice == 4)  // FLTK GUI Mode
        {
            // GUI mode - load processes based on global example set or generate random processes using configured parameters
            scheduler->reset();

            cout << "\nProcess selection for GUI mode:\n";
            cout << "1. Use example process set\n";
            cout << "2. Generate random processes using configured parameters\n";
            cout << "Enter choice (1-2) [default 1]: ";

            string processChoice;
            getline(cin, processChoice);

            if (processChoice == "2")
            {
                // Generate random processes using the configured parameters (from option 6)
                const auto& config = scheduler->getConfig();

                random_device rd;
                mt19937 gen(rd());
                uniform_int_distribution<> arrivalDist(0, config.maxArrivalTime);
                uniform_int_distribution<> burstDist(config.minBurstTime, config.maxBurstTime);

                cout << "\nGenerating " << config.numProcesses << " random processes...\n";
                cout << "  Arrival time range: 0-" << config.maxArrivalTime << " ms\n";
                cout << "  Burst time range: " << config.minBurstTime << "-" << config.maxBurstTime << " ms\n\n";

                for (int i = 0; i < config.numProcesses; i++)
                {
                    int arrival = arrivalDist(gen);
                    int burst = burstDist(gen);
                    scheduler->addProcess(arrival, burst);
                    cout << "  P" << (i+1) << ": Arrival=" << arrival << "ms, Burst=" << burst << "ms\n";
                }

                cout << "\nPress Enter to continue...";
                string temp;
                getline(cin, temp);
            }
            else
            {
                // Default to example set approach
                if (globalExampleSet > 0)
                {
                    loadExampleProcessSet(*scheduler, globalExampleSet);
                    cout << "\nLoaded example process set " << globalExampleSet << ".\n";
                }
                else
                {
                    // Load default example set if no specific set was chosen
                    loadExampleProcessSet(*scheduler);
                }
            }

            runGUIMode(*scheduler);
            continue;
        }
        else if (choice == 5)  // Web GUI Mode
        {
            // Web GUI mode
            scheduler->reset();
            
            cout << "\nProcess selection for Web GUI mode:\n";
            cout << "1. Use example process set\n";
            cout << "2. Generate random processes using configured parameters\n";
            cout << "Enter choice (1-2) [default 1]: ";

            string processChoice;
            getline(cin, processChoice);

            if (processChoice == "2")
            {
                // Generate random processes using the configured parameters
                const auto& config = scheduler->getConfig();

                random_device rd;
                mt19937 gen(rd());
                uniform_int_distribution<> arrivalDist(0, config.maxArrivalTime);
                uniform_int_distribution<> burstDist(config.minBurstTime, config.maxBurstTime);

                cout << "\nGenerating " << config.numProcesses << " random processes...\n";
                cout << "  Arrival time range: 0-" << config.maxArrivalTime << " ms\n";
                cout << "  Burst time range: " << config.minBurstTime << "-" << config.maxBurstTime << " ms\n\n";

                for (int i = 0; i < config.numProcesses; i++)
                {
                    int arrival = arrivalDist(gen);
                    int burst = burstDist(gen);
                    scheduler->addProcess(arrival, burst);
                    cout << "  P" << (i+1) << ": Arrival=" << arrival << "ms, Burst=" << burst << "ms\n";
                }
            }
            else
            {
                // Default to example set approach
                if (globalExampleSet > 0)
                {
                    loadExampleProcessSet(*scheduler, globalExampleSet);
                    cout << "\nLoaded example process set " << globalExampleSet << ".\n";
                }
                else
                {
                    // Load default example set if no specific set was chosen
                    loadExampleProcessSet(*scheduler);
                }
            }

            runWebGUIMode(*scheduler);
            continue;
        }
        else if (choice == 6)  // Algorithm switching mode
        {
            // Algorithm switching mode
            scheduler->reset();

            if (globalExampleSet > 0)
            {
                loadExampleProcessSet(*scheduler, globalExampleSet);
                cout << "\nLoaded example process set " << globalExampleSet << ".\n";
            }
            else
            {
                // Load default example set if no specific set was chosen
                loadExampleProcessSet(*scheduler);
            }

            runAlgorithmSwitchMode(*scheduler, *viz);
            continue;
        }
        else if (choice == 7)  // Configure parameters
        {
            // Configure parameters
            config = ConfigurationManager::configureFromTerminal();
            scheduler = make_unique<MLFQScheduler>(config);
            viz = make_unique<Visualizer>(*scheduler);
            continue;
        }
        else if (choice == 8)  // Load preset configuration
        {
            // Load preset
            displayPresetMenu();
            int preset;
            cin >> preset;
            cin.ignore();
            if (preset >= 1 && preset <= 4)
            {
                config = ConfigurationManager::getPreset(preset);
                scheduler = make_unique<MLFQScheduler>(config);
                viz = make_unique<Visualizer>(*scheduler);
                config.display();
                cout << "\nPress Enter to continue...";
                string temp; getline(cin, temp);
            }
            continue;
        }
        else if (choice == 9)  // Load example process set with default values
        {
            // Load example process set with default values - this sets the global example set
            displayExampleSetsMenu();
            int exampleChoice;
            cin >> exampleChoice;
            cin.ignore(); // Clear newline

            if (exampleChoice >= 1 && exampleChoice <= 5)
            {
                globalExampleSet = exampleChoice;
                cout << "\nGlobal example process set " << exampleChoice << " selected. It will be used for other options.\n";
            }
            else
            {
                cout << "\nInvalid choice! No example set selected.\n";
            }

            continue;
        }
        else if (choice == 10)  // Compare Last Queue Algorithms
        {
            compareLastQueueAlgorithms();
            continue;
        }
        else if (choice == 11)  // Exit
        {
            cout << "Exiting...\n";
            return 0;
        }

        #else

        if (choice == 4)  // Algorithm switching mode
        {
            // Algorithm switching mode
            scheduler->reset();

            if (globalExampleSet > 0)
            {
                loadExampleProcessSet(*scheduler, globalExampleSet);
                cout << "\nLoaded example process set " << globalExampleSet << ".\n";
            }
            else
            {
                // Load default example set if no specific set was chosen
                loadExampleProcessSet(*scheduler);
            }

            runAlgorithmSwitchMode(*scheduler, *viz);
            continue;
        }
        else if (choice == 5)  // Configure parameters
        {
            // Configure parameters
            config = ConfigurationManager::configureFromTerminal();
            scheduler = make_unique<MLFQScheduler>(config);
            viz = make_unique<Visualizer>(*scheduler);
            continue;
        }
        else if (choice == 6)  // Load preset configuration
        {
            // Load preset
            displayPresetMenu();
            int preset;
            cin >> preset;
            cin.ignore();
            if (preset >= 1 && preset <= 4)
            {
                config = ConfigurationManager::getPreset(preset);
                scheduler = make_unique<MLFQScheduler>(config);
                viz = make_unique<Visualizer>(*scheduler);
                config.display();
                cout << "\nPress Enter to continue...";
                string temp2; getline(cin, temp2);
            }
            continue;
        }
        else if (choice == 7)  // Load example process set with default values
        {
            // Load example process set with default values - this sets the global example set
            displayExampleSetsMenu();
            int exampleChoice;
            cin >> exampleChoice;
            cin.ignore(); // Clear newline

            if (exampleChoice >= 1 && exampleChoice <= 5)
            {
                globalExampleSet = exampleChoice;
                cout << "\nGlobal example process set " << exampleChoice << " selected. It will be used for other options.\n";
            }
            else
            {
                cout << "\nInvalid choice! No example set selected.\n";
            }

            continue;
        }
        else if (choice == 8)  // Compare Last Queue Algorithms
        {
            compareLastQueueAlgorithms();
            continue;
        }
        else if (choice == 9)  // Exit
        {
            cout << "Exiting...\n";
            return 0;
        }
        #endif

        {
            scheduler->reset();

            switch (choice)
            {
                case 1:  // Run with example process set
                    if (globalExampleSet > 0)
                    {
                        loadExampleProcessSet(*scheduler, globalExampleSet);
                        cout << "\nLoaded example process set " << globalExampleSet << ".\n";
                    }
                    else
                    {
                        loadExampleProcessSet(*scheduler);
                    }
                    break;
                case 2:  // Create custom processes
                    createCustomProcesses(*scheduler);
                    break;
                case 3:  // Generate random processes
                    generateRandomProcesses(*scheduler);
                    break;
                #ifdef FLTK_AVAILABLE
                case 4:  // If user somehow got here without GUI handling it (shouldn't happen)
                #else
                case 5:  // If user somehow got here without AI handling it (shouldn't happen)
                #endif
                    cout << "Invalid choice sequence!\n";
                    continue;
                default:
                    cout << "Invalid choice!\n";
                    continue;
            }

            // Only ask for execution mode if not using custom or random processes
            // (since they have their own execution flow)
            if (choice != 2 && choice != 3)
            {
                cout << "\nExecution Mode:\n";
                cout << "1. Interactive (step-by-step)\n";
                cout << "2. Automatic (continuous)\n";
                cout << "Enter mode: ";

                int mode;
                // Validate execution mode input
                while (!(cin >> mode))
                {
                    cin.clear(); // Clear error flag
                    cin.ignore(10000, '\n'); // Ignore invalid input
                    cout << "\n" << TerminalUI::Style::error("Invalid input! Please enter 1 or 2: ");
                    cout << "\nExecution Mode:\n";
                    cout << "1. Interactive (step-by-step)\n";
                    cout << "2. Automatic (continuous)\n";
                    cout << "Enter mode: ";
                }
                cin.ignore(); // Clear newline

                if (mode == 1)
                {
                    runInteractiveMode(*scheduler, *viz);
                }
                else if (mode == 2)
                {
                    runAutoMode(*scheduler, *viz);
                }
                else
                {
                    cout << "Invalid mode! Defaulting to Auto mode...\n";
                    runAutoMode(*scheduler, *viz);
                }
            }
            else
            {
                // For custom and random processes, we still offer execution modes
                cout << "\nExecution Mode:\n";
                cout << "1. Interactive (step-by-step)\n";
                cout << "2. Automatic (continuous)\n";
                cout << "Enter mode: ";

                int mode;
                // Validate execution mode input
                while (!(cin >> mode))
                {
                    cin.clear(); // Clear error flag
                    cin.ignore(10000, '\n'); // Ignore invalid input
                    cout << "\n" << TerminalUI::Style::error("Invalid input! Please enter 1 or 2: ");
                    cout << "\nExecution Mode:\n";
                    cout << "1. Interactive (step-by-step)\n";
                    cout << "2. Automatic (continuous)\n";
                    cout << "Enter mode: ";
                }
                cin.ignore(); // Clear newline

                if (mode == 1)
                {
                    runInteractiveMode(*scheduler, *viz);
                }
                else if (mode == 2)
                {
                    runAutoMode(*scheduler, *viz);
                }
                else
                {
                    cout << "Invalid mode! Defaulting to Auto mode...\n";
                    runAutoMode(*scheduler, *viz);
                }
            }

            cout << "\nPress Enter to return to main menu...";
            cin.get();
        }
        // AI mode is handled in the if-else blocks above, so no additional action needed here
    }

    return 0;
}

void runWebGUIMode(MLFQScheduler& scheduler)
{
    cout << "\n" << TerminalUI::Style::header("=== MLFQ Scheduler - Web GUI Mode ===") << "\n";
    
    // Create and start web server
    WebServer webServer(&scheduler, 8080);
    webServer.start();
    
    // Give server time to start
    this_thread::sleep_for(chrono::milliseconds(500));
    
    // Automatically open browser
    cout << "Opening browser automatically...\n";
    system("xdg-open http://localhost:8080 2>/dev/null || open http://localhost:8080 2>/dev/null || start http://localhost:8080 2>/dev/null");
    
    cout << "\nPress Enter to stop the web server and return to main menu...";
    cin.get();
    
    webServer.stop();
    cout << "\nWeb server stopped.\n";
}
