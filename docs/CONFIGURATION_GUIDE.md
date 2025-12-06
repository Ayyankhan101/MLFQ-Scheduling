# MLFQ Scheduler Configuration Guide

## Overview

The MLFQ Scheduler supports comprehensive runtime configuration, allowing you to customize algorithm parameters, process characteristics, and visualization settings through terminal interfaces. The system also supports algorithm switching for the last queue level.

## Configuration Parameters

### Core MLFQ Algorithm Parameters

1. **Number of Queues** (2-5)
   - Default: 3
   - Controls how many priority levels exist
   - More queues = finer-grained priority control
   - Fewer queues = simpler, more predictable behavior

2. **Base Time Quantum** (2-10 ms)
   - Default: 4 ms
   - Time slice for the highest priority queue (Queue 0)
   - Lower values = more responsive to interactive processes
   - Higher values = better throughput for CPU-bound processes

3. **Quantum Growth Factor** (1.0-5.0x)
   - Default: 2.0x
   - How much the time quantum increases for each lower priority queue
   - Formula: `Quantum(queue_n) = base_quantum × (multiplier^n)`
   - Examples:
     - Base=4, Multiplier=2.0: Q0=4ms, Q1=8ms, Q2=16ms
     - Base=2, Multiplier=1.5: Q0=2ms, Q1=3ms, Q2=4.5ms, Q3=6.75ms

4. **Priority Boost Interval** (20-500 ms)
   - Default: 100 ms
   - Time period after which all processes are moved to highest priority
   - Prevents starvation of low-priority processes
   - Lower values = more aggressive anti-starvation
   - Higher values = more strict priority enforcement

### Process Generation Parameters

5. **Number of Processes** (1-20)
   - Default: 5
   - Used for random process generation

6. **Maximum Arrival Time** (0-100 ms)
   - Default: 20 ms
   - Window during which processes can arrive
   - 0 = all processes arrive simultaneously
   - Higher values = more spread-out arrivals

7. **Burst Time Range** (1-100 ms)
   - Min Default: 5 ms
   - Max Default: 30 ms
   - CPU execution time for randomly generated processes

### Visualization Parameters

8. **Animation Delay** (50-2000 ms)
   - Default: 200 ms
   - Time between execution steps in automatic mode
   - Lower values = faster visualization
   - Higher values = easier to observe behavior

### Last Queue Algorithm

9. **Algorithm for Last Queue**
   - Options: Round Robin, Shortest Job First (SJF), Priority Scheduling
   - Only available in algorithm-switching mode (menu option 5)
   - Allows different scheduling strategies for low-priority processes

## Configuration Methods

### Method 1: Interactive Configuration (Terminal)

From the main menu, select option **6** (Configure scheduler parameters):

```
Enter choice: 6
```

You'll be prompted for each parameter:
```
Number of queues [2-5] [3]: 4
Base time quantum (ms) [2-10] [4]: 6
Quantum growth factor [1.5-5.0] [2.0]: 1.5
Priority boost interval (ms) [20-500] [100]: 50
Animation delay (ms) [50-2000] [200]: 100
Number of processes [1-20] [5]: 8
Max arrival time (ms) [0-100] [20]: 15
Min burst time (ms) [1-100] [5]: 3
Max burst time (ms) [1-100] [30]: 25
```

Press Enter to accept default values (shown in brackets).

### Method 2: Preset Configurations

From the main menu, select option **7** (Load preset configuration):

#### Preset 1: Standard MLFQ
- **Use Case**: General-purpose scheduling
- **Parameters**:
  - Queues: 3
  - Base Quantum: 4 ms
  - Multiplier: 2.0x
  - Boost Interval: 100 ms
- **Behavior**: Balanced between responsiveness and throughput

#### Preset 2: Fine-Grained
- **Use Case**: Interactive systems, many short processes
- **Parameters**:
  - Queues: 5
  - Base Quantum: 2 ms
  - Multiplier: 1.5x
  - Boost Interval: 50 ms
- **Behavior**: Very responsive, frequent priority adjustments

#### Preset 3: Coarse-Grained
- **Use Case**: CPU-intensive batch processing
- **Parameters**:
  - Queues: 2
  - Base Quantum: 8 ms
  - Multiplier: 3.0x
  - Boost Interval: 200 ms
- **Behavior**: Favors long-running processes, less overhead

#### Preset 4: Aggressive Anti-Starvation
- **Use Case**: Real-time systems, prevent any process starvation
- **Parameters**:
  - Queues: 3
  - Base Quantum: 4 ms
  - Multiplier: 2.0x
  - Boost Interval: 30 ms
- **Behavior**: Very frequent priority boosts, fair time distribution

### Method 3: Algorithm-Switching Mode

From the main menu, select option **5** (Run with algorithm-switching scheduler):

1. Select algorithm for last queue:
   - 1. Round Robin (standard behavior)
   - 2. Shortest Job First (processes with shortest remaining time execute first)
   - 3. Priority Scheduling (processes with highest priority execute first)

2. Choose execution mode (Interactive/Automatic)

## Configuration Examples

### Example 1: Testing Starvation Prevention

**Goal**: See how boost interval affects low-priority processes

```
Configure Parameters:
- Queues: 3
- Base Quantum: 4
- Multiplier: 2.0
- Boost Interval: 20 (aggressive) vs 200 (relaxed)

Load Process Set: 2 (CPU-Intensive)
```

**Observation**: With boost=20, all processes complete more evenly. With boost=200, early processes may starve later arrivals.

### Example 2: Interactive vs Batch Workloads

**Goal**: Compare fine-grained vs coarse-grained scheduling

```
Test 1 (Interactive):
- Use Preset 2 (Fine-Grained)
- Generate processes with short burst times (5-15 ms)
- Observe: Lower average response time

Test 2 (Batch):
- Use Preset 3 (Coarse-Grained)
- Generate processes with long burst times (20-50 ms)
- Observe: Higher throughput, lower context switch overhead
```

### Example 3: Last Queue Algorithm Comparison

**Goal**: Compare different algorithms on the last queue

```
1. Load same process set (e.g., example set 1)
2. Use same configuration parameters
3. Run with each last queue algorithm:
   - Round Robin (default)
   - Shortest Job First
   - Priority Scheduling
4. Compare metrics
```

**Observation**: Different algorithms can significantly impact performance of long-running processes in the lowest priority queue.

## How Configuration Affects Behavior

### Number of Queues

| Queues | Pros | Cons |
|--------|------|------|
| 2 | Simple, low overhead | Less granular priority control |
| 3 (default) | Good balance | Standard complexity |
| 4-5 | Fine-grained control | More complexity, more queue management |

### Time Quantum

| Setting | Effect on System |
|---------|------------------|
| Small (2-4 ms) | More responsive, higher overhead |
| Medium (4-8 ms) | Balanced |
| Large (8-10 ms) | Better throughput, less responsive |

### Boost Interval

| Setting | Starvation Risk | Priority Enforcement |
|---------|----------------|---------------------|
| Low (20-50 ms) | Very low | Weak |
| Medium (50-150 ms) | Low | Moderate |
| High (150-500 ms) | Moderate | Strong |

## Algorithm-Switching Benefits

### Round Robin (RR) for Last Queue
- Default behavior
- Fair time sharing among low-priority processes
- Predictable behavior

### Shortest Job First (SJF) for Last Queue
- Minimizes average waiting time
- Good for batch processing
- Potential starvation risk for long jobs

### Priority Scheduling for Last Queue
- Prioritizes processes based on characteristics
- Can optimize for specific metrics
- Requires careful priority assignment

## Tips and Best Practices

1. **Start with presets** - Use preset configurations to understand different behaviors before customizing

2. **Incremental changes** - Change one parameter at a time to understand its impact

3. **Match workload** - Configure based on your process characteristics:
   - Short, interactive: Fine-grained preset
   - Long, CPU-bound: Coarse-grained preset
   - Mixed: Standard preset

4. **Monitor metrics** - Watch the statistics display for:
   - Average wait time
   - Average turnaround time
   - CPU utilization
   - Response time

5. **Test different algorithms** - Try algorithm-switching mode to see how different strategies affect the last queue

6. **Test edge cases** - Try extreme values to understand algorithm limits

## Validation

All configuration parameters are validated:
- Invalid values are rejected with error messages
- Default values are used if input is invalid
- Range checks ensure parameters stay within reasonable bounds

## Results Export

When running in automatic mode, you can export results to CSV format:
- Process details (PID, arrival, burst, completion times)
- Individual metrics (turnaround, wait, response times)
- Summary statistics
- Timestamped filename for easy identification

## Saving Configurations

Currently, configurations are session-based. To reuse a configuration:
1. Note your parameter values
2. Use "Configure parameters" option (menu 6)
3. Re-enter the values
4. Or use "Load preset configuration" option (menu 7) for standard configurations





