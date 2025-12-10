class MLFQWebInterface {
    constructor() {
        this.processes = [];
        this.queues = [
            { id: 0, name: 'Queue 0 (Highest Priority)', quantum: 1, processes: [] },
            { id: 1, name: 'Queue 1', quantum: 2, processes: [] },
            { id: 2, name: 'Queue 2', quantum: 4, processes: [] },
            { id: 3, name: 'Queue 3 (Lowest Priority)', quantum: 8, processes: [] }
        ];
        this.currentTime = 0;
        this.isRunning = false;
        this.speed = 5;
        this.intervalId = null;
        this.lastQueueAlgorithm = 'rr';
        this.priorityBoost = true;
        this.boostInterval = 20;
        this.lastBoostTime = 0;
        this.nextBoostTime = 20;
        this.currentQuantumTime = 0;  // Track current quantum usage
        this.currentRunningProcess = null;  // Track which process is running
        
        this.initializeEventListeners();
        this.renderQueues();
        this.renderProcessTable();
        this.updateMetrics();
        this.updateBoostStatus();
        this.updateTimeDisplay();
    }

    updateBoostStatus() {
        const indicator = document.getElementById('boost-indicator');
        const timestamp = document.getElementById('boost-timestamp');
        const countdown = document.getElementById('boost-countdown');
        const now = new Date().toLocaleString();
        
        if (this.priorityBoost) {
            indicator.textContent = 'Boost Priority: ON';
            indicator.className = 'boost-enabled';
            timestamp.textContent = `Last boost: Time ${this.lastBoostTime}`;
            
            if (this.isRunning) {
                const timeToNext = this.nextBoostTime - this.currentTime;
                countdown.textContent = `Next boost in: ${Math.max(0, timeToNext)} time units`;
            } else {
                countdown.textContent = `Interval: ${this.boostInterval} time units`;
            }
        } else {
            indicator.textContent = 'Boost Priority: OFF';
            indicator.className = 'boost-disabled';
            timestamp.textContent = `Disabled at: ${now}`;
            countdown.textContent = '';
        }
    }

    updateTimeDisplay() {
        const timeElement = document.getElementById('current-time');
        timeElement.textContent = `Time: ${this.currentTime} ms`;
    }

    initializeEventListeners() {
        // Control buttons
        document.getElementById('startBtn').addEventListener('click', () => this.startSimulation());
        document.getElementById('pauseBtn').addEventListener('click', () => this.pauseSimulation());
        document.getElementById('resetBtn').addEventListener('click', () => this.resetSimulation());
        document.getElementById('stepBtn').addEventListener('click', () => this.stepSimulation());
        document.getElementById('configBtn').addEventListener('click', () => this.showConfigModal());
        
        // Speed control
        document.getElementById('speed').addEventListener('input', (e) => {
            this.speed = parseInt(e.target.value);
            document.getElementById('speedValue').textContent = `${this.speed}x`;
            if (this.isRunning) {
                this.pauseSimulation();
                this.startSimulation();
            }
        });
        
        // Process management
        document.getElementById('addProcessBtn').addEventListener('click', () => this.showProcessModal());
        document.getElementById('loadExampleBtn').addEventListener('click', () => this.loadPreset('basic'));
        
        // Modal handling
        document.querySelectorAll('.close').forEach(btn => {
            btn.addEventListener('click', (e) => {
                e.target.closest('.modal').style.display = 'none';
            });
        });
        
        window.addEventListener('click', (e) => {
            if (e.target.classList.contains('modal')) {
                e.target.style.display = 'none';
            }
        });
        
        // Form submissions
        document.getElementById('processForm').addEventListener('submit', (e) => {
            e.preventDefault();
            this.addProcess();
        });
        
        // Config modal tabs
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.addEventListener('click', (e) => this.switchTab(e.target.dataset.tab));
        });
        
        // Config actions
        document.querySelectorAll('.preset-btn').forEach(btn => {
            btn.addEventListener('click', (e) => this.loadPreset(e.target.dataset.preset));
        });
        
        document.getElementById('loadCustomBtn').addEventListener('click', () => this.loadCustomSet());
        document.getElementById('generateRandomBtn').addEventListener('click', () => this.generateRandomSet());
        document.getElementById('applyQueueConfigBtn').addEventListener('click', () => this.applyQueueConfig());
        document.getElementById('applyAlgorithmBtn').addEventListener('click', () => this.applyAlgorithmConfig());
        
        // Quantum mode toggle
        document.getElementById('quantumMode').addEventListener('change', (e) => {
            const customDiv = document.getElementById('customQuantums');
            customDiv.style.display = e.target.value === 'custom' ? 'block' : 'none';
        });
    }

    showProcessModal() {
        document.getElementById('processModal').style.display = 'block';
    }

    showConfigModal() {
        document.getElementById('configModal').style.display = 'block';
    }

    switchTab(tabName) {
        document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
        document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));
        
        document.querySelector(`[data-tab="${tabName}"]`).classList.add('active');
        document.getElementById(tabName).classList.add('active');
    }

    loadPreset(presetName) {
        this.processes = [];
        
        const presets = {
            basic: [
                { id: 'P1', arrivalTime: 0, burstTime: 8 },
                { id: 'P2', arrivalTime: 1, burstTime: 4 },
                { id: 'P3', arrivalTime: 2, burstTime: 9 },
                { id: 'P4', arrivalTime: 3, burstTime: 5 }
            ],
            mixed: [
                { id: 'P1', arrivalTime: 0, burstTime: 6 },
                { id: 'P2', arrivalTime: 2, burstTime: 8 },
                { id: 'P3', arrivalTime: 4, burstTime: 3 },
                { id: 'P4', arrivalTime: 6, burstTime: 4 },
                { id: 'P5', arrivalTime: 8, burstTime: 10 },
                { id: 'P6', arrivalTime: 10, burstTime: 2 }
            ],
            heavy: [
                { id: 'P1', arrivalTime: 0, burstTime: 12 },
                { id: 'P2', arrivalTime: 1, burstTime: 8 },
                { id: 'P3', arrivalTime: 2, burstTime: 15 },
                { id: 'P4', arrivalTime: 3, burstTime: 6 },
                { id: 'P5', arrivalTime: 4, burstTime: 9 },
                { id: 'P6', arrivalTime: 5, burstTime: 11 },
                { id: 'P7', arrivalTime: 6, burstTime: 7 },
                { id: 'P8', arrivalTime: 7, burstTime: 13 }
            ],
            interactive: [
                { id: 'UI1', arrivalTime: 0, burstTime: 3 },
                { id: 'BG1', arrivalTime: 1, burstTime: 12 },
                { id: 'UI2', arrivalTime: 4, burstTime: 2 },
                { id: 'BG2', arrivalTime: 6, burstTime: 8 },
                { id: 'UI3', arrivalTime: 8, burstTime: 4 }
            ]
        };
        
        const preset = presets[presetName] || presets.basic;
        preset.forEach(p => {
            this.processes.push({
                ...p,
                remainingTime: p.burstTime,
                priority: 0,
                state: 'NEW',
                waitTime: 0,
                turnaroundTime: 0,
                responseTime: -1,
                completionTime: 0,
                currentQueue: 0
            });
        });
        
        this.resetSimulation();
        document.getElementById('configModal').style.display = 'none';
    }

    loadCustomSet() {
        const numProcesses = parseInt(document.getElementById('customNumProcesses').value);
        const baseArrival = parseInt(document.getElementById('baseArrival').value);
        const arrivalStep = parseInt(document.getElementById('arrivalStep').value);
        const avgBurst = parseInt(document.getElementById('avgBurst').value);
        
        this.processes = [];
        
        for (let i = 1; i <= numProcesses; i++) {
            const arrivalTime = baseArrival + (i - 1) * arrivalStep;
            const burstTime = Math.max(2, avgBurst + Math.floor(Math.random() * 5) - 2); // ±2 variation
            
            this.processes.push({
                id: `P${i}`,
                arrivalTime,
                burstTime,
                remainingTime: burstTime,
                priority: 0,
                state: 'NEW',
                waitTime: 0,
                turnaroundTime: 0,
                responseTime: -1,
                completionTime: 0,
                currentQueue: 0
            });
        }
        
        this.resetSimulation();
        document.getElementById('configModal').style.display = 'none';
    }

    generateRandomSet() {
        const numProcesses = parseInt(document.getElementById('numProcesses').value);
        const maxArrival = parseInt(document.getElementById('maxArrival').value);
        const minBurst = parseInt(document.getElementById('minBurst').value);
        const maxBurst = parseInt(document.getElementById('maxBurst').value);
        
        this.processes = [];
        
        for (let i = 1; i <= numProcesses; i++) {
            const arrivalTime = Math.floor(Math.random() * (maxArrival + 1));
            const burstTime = Math.floor(Math.random() * (maxBurst - minBurst + 1)) + minBurst;
            
            this.processes.push({
                id: `P${i}`,
                arrivalTime,
                burstTime,
                remainingTime: burstTime,
                priority: 0,
                state: 'NEW',
                waitTime: 0,
                turnaroundTime: 0,
                responseTime: -1,
                completionTime: 0,
                currentQueue: 0
            });
        }
        
        // Sort by arrival time
        this.processes.sort((a, b) => a.arrivalTime - b.arrivalTime);
        
        this.resetSimulation();
        document.getElementById('configModal').style.display = 'none';
    }

    applyQueueConfig() {
        const numQueues = parseInt(document.getElementById('numQueues').value);
        const quantumMode = document.getElementById('quantumMode').value;
        
        let quantums = [];
        
        if (quantumMode === 'custom') {
            // Parse custom quantum values
            const quantumValues = document.getElementById('quantumValues').value;
            quantums = quantumValues.split(',').map(q => parseInt(q.trim())).filter(q => !isNaN(q));
            
            // Ensure we have enough quantums for all queues
            while (quantums.length < numQueues) {
                quantums.push(quantums[quantums.length - 1] || 1);
            }
        } else {
            // Auto mode: exponential increase (1, 2, 4, 8...)
            for (let i = 0; i < numQueues; i++) {
                quantums.push(Math.pow(2, i));
            }
        }
        
        // Recreate queues array
        this.queues = [];
        for (let i = 0; i < numQueues; i++) {
            this.queues.push({
                id: i,
                name: i === 0 ? 'Queue 0 (Highest Priority)' : 
                      i === numQueues - 1 ? `Queue ${i} (Lowest Priority)` : 
                      `Queue ${i}`,
                quantum: quantums[i],
                processes: []
            });
        }
        
        this.resetSimulation();
        document.getElementById('configModal').style.display = 'none';
    }

    addProcess() {
        const processId = document.getElementById('processId').value;
        const arrivalTime = parseInt(document.getElementById('arrivalTime').value);
        const burstTime = parseInt(document.getElementById('burstTime').value);
        const priority = parseInt(document.getElementById('priority').value);
        
        if (!processId || isNaN(arrivalTime) || isNaN(burstTime)) {
            alert('Please fill all required fields with valid values');
            return;
        }
        
        const newProcess = {
            id: processId,
            arrivalTime: arrivalTime,
            burstTime: burstTime,
            remainingTime: burstTime,
            priority: priority || 0,
            state: 'NEW',
            waitTime: 0,
            turnaroundTime: 0,
            responseTime: -1,
            completionTime: 0,
            currentQueue: 0
        };
        
        // Check if process with same ID already exists
        const existingIndex = this.processes.findIndex(p => p.id === processId);
        
        if (existingIndex !== -1) {
            // Replace existing process
            this.processes[existingIndex] = newProcess;
        } else {
            // Add new process at the end
            this.processes.push(newProcess);
        }
        
        this.renderProcessTable();
        document.getElementById('processModal').style.display = 'none';
        document.getElementById('processForm').reset();
    }

    startSimulation() {
        if (this.processes.length === 0) {
            alert('Please add processes first! Use the Config button to load a process set.');
            return;
        }
        
        this.isRunning = true;
        document.getElementById('startBtn').disabled = true;
        document.getElementById('pauseBtn').disabled = false;
        
        const interval = Math.max(100, 1000 / this.speed);
        this.intervalId = setInterval(() => this.simulationStep(), interval);
    }

    pauseSimulation() {
        this.isRunning = false;
        document.getElementById('startBtn').disabled = false;
        document.getElementById('pauseBtn').disabled = true;
        
        if (this.intervalId) {
            clearInterval(this.intervalId);
            this.intervalId = null;
        }
    }

    resetSimulation() {
        this.pauseSimulation();
        this.currentTime = 0;
        this.lastBoostTime = 0;
        this.nextBoostTime = this.boostInterval;
        this.currentQuantumTime = 0;
        this.currentRunningProcess = null;
        
        // Reset all processes
        this.processes.forEach(process => {
            process.remainingTime = process.burstTime;
            process.state = 'NEW';
            process.waitTime = 0;
            process.turnaroundTime = 0;
            process.responseTime = -1;
            process.completionTime = 0;
            process.currentQueue = 0;
        });
        
        // Clear all queues
        this.queues.forEach(queue => {
            queue.processes = [];
        });
        
        this.renderQueues();
        this.renderProcessTable();
        this.updateMetrics();
        this.updateTimeDisplay();
    }

    stepSimulation() {
        this.simulationStep();
    }

    simulationStep() {
        // Check if all processes are terminated - stop simulation immediately
        if (this.processes.length > 0 && this.processes.every(p => p.state === 'TER')) {
            this.pauseSimulation();
            return;
        }

        // Check for priority boost
        if (this.priorityBoost && this.currentTime > 0 && this.currentTime % this.boostInterval === 0) {
            this.performPriorityBoost();
        }

        // Check for new arrivals
        this.processes.forEach(process => {
            if (process.arrivalTime === this.currentTime && process.state === 'NEW') {
                process.state = 'READY';
                this.insertIntoLastQueue(process, 0); // Start at highest priority queue
            }
        });

        // Execute processes
        let executed = false;
        let runningProcess = null;
        
        for (let i = 0; i < this.queues.length && !executed; i++) {
            const queue = this.queues[i];
            if (queue.processes.length > 0) {
                const process = queue.processes[0];
                runningProcess = process;
                
                // Check if this is a new process starting or continuing
                if (this.currentRunningProcess !== process) {
                    this.currentRunningProcess = process;
                    this.currentQuantumTime = 0;
                }
                
                if (process.responseTime === -1) {
                    process.responseTime = this.currentTime - process.arrivalTime;
                }
                
                process.state = 'RUNNING';
                process.remainingTime--;
                this.currentQuantumTime++;
                executed = true;
                
                // Check if process is complete
                if (process.remainingTime === 0) {
                    process.state = 'TERMINATED';
                    process.completionTime = this.currentTime + 1;
                    process.turnaroundTime = process.completionTime - process.arrivalTime;
                    process.waitTime = process.turnaroundTime - process.burstTime;
                    queue.processes.shift();
                    this.currentRunningProcess = null;
                    this.currentQuantumTime = 0;
                    runningProcess = null;
                } else {
                    // Check if quantum expired
                    const currentQuantum = this.queues[i].quantum;
                    if (this.currentQuantumTime >= currentQuantum) {
                        // Quantum expired - move to next queue or round robin
                        if (i < this.queues.length - 1) {
                            // Move to lower priority queue
                            process.state = 'READY';
                            process.currentQueue = i + 1;
                            queue.processes.shift();
                            this.insertIntoLastQueue(process, i + 1);
                        } else {
                            // Round robin in lowest queue
                            process.state = 'READY';
                            queue.processes.shift();
                            queue.processes.push(process);
                        }
                        this.currentRunningProcess = null;
                        this.currentQuantumTime = 0;
                        runningProcess = null;
                    }
                    // If quantum not expired, process continues running
                }
            }
        }

        // Update wait times for waiting processes
        this.processes.forEach(process => {
            if (process.state === 'READY') {
                process.waitTime++;
            }
        });

        this.currentTime++;
        this.renderQueues();
        this.renderProcessTable();
        this.updateMetrics();
        this.updateBoostStatus();
        this.updateTimeDisplay();
        
        // Reset running process state after display
        if (runningProcess && runningProcess.state === 'RUNNING' && runningProcess.remainingTime > 0) {
            runningProcess.state = 'READY';
        }

        // Check if simulation is complete
        if (this.processes.every(p => p.state === 'TERMINATED')) {
            this.pauseSimulation();
            alert('Simulation completed!');
        }
    }

    performPriorityBoost() {
        // Move all processes from lower priority queues to highest priority queue
        let boostedCount = 0;
        
        for (let i = 1; i < this.queues.length; i++) {
            const processes = this.queues[i].processes.splice(0);
            processes.forEach(process => {
                process.currentQueue = 0;
                this.queues[0].processes.push(process);
                boostedCount++;
            });
        }
        
        this.lastBoostTime = this.currentTime;
        this.nextBoostTime = this.currentTime + this.boostInterval;
        
        // Reset quantum tracking for boost
        this.currentQuantumTime = 0;
        this.currentRunningProcess = null;
        
        if (boostedCount > 0) {
            console.log(`Priority boost at time ${this.currentTime}: ${boostedCount} processes moved to highest priority queue`);
        }
    }

    insertIntoLastQueue(process, queueIndex) {
        const queue = this.queues[queueIndex];
        
        // If not the last queue, just add normally
        if (queueIndex < this.queues.length - 1) {
            queue.processes.push(process);
            return;
        }
        
        // Last queue - apply selected algorithm
        switch(this.lastQueueAlgorithm) {
            case 'sjf': // Shortest Job First
                // Insert based on remaining time (shortest first)
                let sjfIndex = 0;
                while (sjfIndex < queue.processes.length && 
                       queue.processes[sjfIndex].remainingTime <= process.remainingTime) {
                    sjfIndex++;
                }
                queue.processes.splice(sjfIndex, 0, process);
                break;
                
            case 'fcfs': // First Come First Serve
                // Insert based on arrival time (earliest first)
                let fcfsIndex = 0;
                while (fcfsIndex < queue.processes.length && 
                       queue.processes[fcfsIndex].arrivalTime <= process.arrivalTime) {
                    fcfsIndex++;
                }
                queue.processes.splice(fcfsIndex, 0, process);
                break;
                
            case 'priority': // Priority Scheduling (longest wait time first)
                // Insert based on wait time (longest waiting first)
                let priorityIndex = 0;
                while (priorityIndex < queue.processes.length && 
                       queue.processes[priorityIndex].waitTime >= process.waitTime) {
                    priorityIndex++;
                }
                queue.processes.splice(priorityIndex, 0, process);
                break;
                
            case 'rr': // Round Robin (default)
            default:
                // Add to end for round robin
                queue.processes.push(process);
                break;
        }
    }

    renderQueues() {
        const container = document.getElementById('queues');
        container.innerHTML = '';
        
        this.queues.forEach((queue, index) => {
            const queueDiv = document.createElement('div');
            queueDiv.className = 'queue';
            
            queueDiv.innerHTML = `
                <div class="queue-header">
                    <span class="queue-title">${queue.name}</span>
                    <span class="quantum-info">Quantum: ${queue.quantum}</span>
                </div>
                <div class="process-list">
                    ${queue.processes.map(p => 
                        `<div class="process-item ${p.state === 'RUNNING' ? 'running' : ''}">${p.id}</div>`
                    ).join('')}
                </div>
            `;
            
            container.appendChild(queueDiv);
        });
    }

    renderProcessTable() {
        const container = document.getElementById('processTable');
        
        const table = document.createElement('table');
        table.innerHTML = `
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Arrival</th>
                    <th>Burst</th>
                    <th>Remaining</th>
                    <th>State</th>
                    <th>Completion Time</th>
                    <th>Wait Time</th>
                    <th>Turn around</th>
                </tr>
            </thead>
            <tbody>
                ${this.processes.map(p => {
                    // Calculate turnaround time as: Completion Time - Arrival Time
                    const turnaroundTime = p.state === 'TERMINATED' ? p.completionTime - p.arrivalTime : 0;
                    const completionTime = p.state === 'TERMINATED' ? p.completionTime : '-';
                    
                    return `
                        <tr>
                            <td>${p.id}</td>
                            <td>${p.arrivalTime}</td>
                            <td>${p.burstTime}</td>
                            <td>${p.remainingTime}</td>
                            <td>${p.state}</td>
                            <td>${completionTime}</td>
                            <td>${p.waitTime}</td>
                            <td>${turnaroundTime}</td>
                        </tr>
                    `;
                }).join('')}
            </tbody>
        `;
        
        container.innerHTML = '';
        container.appendChild(table);
    }

    updateMetrics() {
        const completedProcesses = this.processes.filter(p => p.state === 'TERMINATED');
        
        if (completedProcesses.length > 0) {
            // Calculate metrics correctly using proper formulas
            let totalWaitTime = 0;
            let totalTurnaroundTime = 0;
            let totalResponseTime = 0;
            
            completedProcesses.forEach(p => {
                // Correct formulas:
                const turnaroundTime = p.completionTime - p.arrivalTime;  // Completion - Arrival
                const waitTime = turnaroundTime - p.burstTime;            // Turnaround - Burst
                const responseTime = p.responseTime;                      // Already calculated correctly
                
                totalTurnaroundTime += turnaroundTime;
                totalWaitTime += waitTime;
                totalResponseTime += responseTime;
            });
            
            const avgWaitTime = totalWaitTime / completedProcesses.length;
            const avgTurnaround = totalTurnaroundTime / completedProcesses.length;
            const avgResponse = totalResponseTime / completedProcesses.length;
            
            // Throughput: processes completed per unit time
            const throughput = this.currentTime > 0 ? (completedProcesses.length / this.currentTime) : 0;
            
            // CPU Utilization: total burst time of completed processes / total elapsed time
            const totalBurstTime = completedProcesses.reduce((sum, p) => sum + p.burstTime, 0);
            const cpuUtil = this.currentTime > 0 ? (totalBurstTime / this.currentTime * 100) : 0;
            
            document.getElementById('avgWaitTime').textContent = avgWaitTime.toFixed(2);
            document.getElementById('avgTurnaround').textContent = avgTurnaround.toFixed(2);
            document.getElementById('throughput').textContent = throughput.toFixed(3);
            document.getElementById('cpuUtil').textContent = Math.min(100, cpuUtil).toFixed(1) + '%';
        } else {
            // Reset metrics when no processes completed
            document.getElementById('avgWaitTime').textContent = '0.00';
            document.getElementById('avgTurnaround').textContent = '0.00';
            document.getElementById('throughput').textContent = '0.000';
            document.getElementById('cpuUtil').textContent = '0.0%';
        }
    }

    applyAlgorithmConfig() {
        this.lastQueueAlgorithm = document.getElementById('lastQueueAlgorithm').value;
        this.priorityBoost = document.getElementById('priorityBoost').value === 'enabled';
        this.boostInterval = parseInt(document.getElementById('boostInterval').value);
        this.nextBoostTime = this.currentTime + this.boostInterval;
        
        this.updateBoostStatus();
        document.getElementById('configModal').style.display = 'none';
        alert('Algorithm configuration applied!');
    }
}

// Initialize the application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new MLFQWebInterface();
});
