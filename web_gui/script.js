class MLFQWebInterface 
{
    constructor() 
    {
        this.isRunning = false;
        this.intervalId = null;
        this.updateIntervalId = null;
        this.initializeEventListeners();
        this.updateDisplay();
        this.startAutoUpdate();
    }

    startAutoUpdate() 
    {
        if (this.updateIntervalId) 
        {
            clearInterval(this.updateIntervalId);
        }
        this.updateIntervalId = setInterval(() => this.updateDisplay(), 500);
    }

    stopAutoUpdate() 
    {
        if (this.updateIntervalId) 
        {
            clearInterval(this.updateIntervalId);
            this.updateIntervalId = null;
        }
    }

    initializeEventListeners() 
    {
        document.getElementById('start-btn').onclick = () => this.startSimulation();
        document.getElementById('pause-btn').onclick = () => this.pauseSimulation();
        document.getElementById('step-btn').onclick = () => this.stepSimulation();
        document.getElementById('reset-btn').onclick = () => this.resetSimulation();
        document.getElementById('preset-btn').onclick = () => this.showPreset();
        document.getElementById('random-btn').onclick = () => this.showRandom();
        document.getElementById('config-btn').onclick = () => this.showConfig();
        document.getElementById('add-process-btn').onclick = () => this.showAddProcess();
        
        // Close modals when clicking outside
        document.getElementById('config-modal').onclick = (e) => 
        {
            if (e.target.id === 'config-modal') this.closeConfig();
        };
        document.getElementById('add-process-modal').onclick = (e) => {
            if (e.target.id === 'add-process-modal') this.closeAddProcess();
        };
        document.getElementById('preset-modal').onclick = (e) => {
            if (e.target.id === 'preset-modal') this.closePreset();
        };
        document.getElementById('random-modal').onclick = (e) => {
            if (e.target.id === 'random-modal') this.closeRandom();
        };
        
        // ESC key to close modals
        document.addEventListener('keydown', (e) => 
        {
            if (e.key === 'Escape') 
            {
                this.closeConfig();
                this.closeAddProcess();
                this.closePreset();
                this.closeRandom();
            }
        });
    }

    async updateDisplay() {
        try {
            // Update status
            const statusResponse = await fetch('/api/status');
            const statusData = await statusResponse.json();
            
            document.getElementById('current-time').textContent = statusData.time;
            document.getElementById('active-processes').textContent = statusData.totalProcesses - statusData.completedProcesses;
            document.getElementById('completed-processes').textContent = statusData.completedProcesses;
            document.getElementById('cpu-utilization').textContent = statusData.cpuUtilization.toFixed(1) + '%';
            document.getElementById('avg-wait-time').textContent = statusData.avgWaitTime.toFixed(1);
            document.getElementById('avg-turnaround-time').textContent = statusData.avgTurnaroundTime.toFixed(1);
            document.getElementById('avg-response-time').textContent = statusData.avgResponseTime.toFixed(1);
            
            // Update boost information
            if (statusData.boostEnabled) {
                document.getElementById('boost-status').textContent = 'Priority Boost: ON';
                document.getElementById('boost-timestamp').textContent = `Last boost: Time ${statusData.lastBoostTime}`;
                document.getElementById('boost-countdown').textContent = `Next boost in: ${statusData.nextBoostIn} time units`;
            } else {
                document.getElementById('boost-status').textContent = 'Priority Boost: OFF';
                document.getElementById('boost-timestamp').textContent = '';
                document.getElementById('boost-countdown').textContent = '';
            }
            
            // Auto-stop if running when no processes left, but keep buttons enabled
            // so user can add new processes and then start/step
            if (!statusData.hasProcesses) {
                if (this.isRunning) {
                    this.pauseSimulation();
                }
            }

            // Check if simulation is complete (all processes terminated) to disable buttons appropriately
            if (statusData.isComplete === true) {
                // All processes are completed - disable step/start buttons
                document.getElementById('step-btn').disabled = true;
                document.getElementById('start-btn').disabled = true;
            } else if (statusData.processesExist === true) {
                // Processes exist in the system (even if not yet available) - keep buttons enabled
                document.getElementById('step-btn').disabled = false;
                document.getElementById('start-btn').disabled = false;
            } else {
                // No processes exist in the system - disable buttons
                document.getElementById('step-btn').disabled = true;
                document.getElementById('start-btn').disabled = true;
            }
            
            // Update process table
            await this.updateProcessTable();
            await this.updateQueues();
            
        } catch (e) {
            console.log('Backend not available');
        }
    }

    async updateProcessTable() {
        try {
            const response = await fetch('/api/processes');
            const data = await response.json();

            const tbody = document.getElementById('process-table-body');
            tbody.innerHTML = '';

            data.processes.forEach(process => {
                const row = tbody.insertRow();
                // Add special class for running process
                const isRunning = process.pid === data.currentRunningPid;
                const rowClass = isRunning ? 'running-process-table-row' : '';

                row.className = rowClass;
                row.innerHTML = `
                    <td>P${process.pid}</td>
                    <td>${process.arrival}</td>
                    <td>${process.burst}</td>
                    <td>${process.remaining}</td>
                    <td>${process.completion === 0 ? '-' : process.completion}</td>
                    <td><span class="status-${process.status.toLowerCase()}">${process.status}</span></td>
                `;
            });
        } catch (e) {
            console.log('Failed to update process table');
        }
    }

    async updateQueues() {
        try {
            // Get both queue and process information
            const [queuesResponse, processesResponse] = await Promise.all([
                fetch('/api/queues'),
                fetch('/api/processes')
            ]);

            const queuesData = await queuesResponse.json();
            const processesData = await processesResponse.json();

            const container = document.getElementById('queues-container');
            container.innerHTML = '';

            // Create a map of process PID to process object for quick lookup
            const processMap = {};
            processesData.processes.forEach(process => {
                processMap[process.pid] = process;
            });

            queuesData.queues.forEach((queue, index) => {
                const queueDiv = document.createElement('div');
                queueDiv.className = 'queue';

                // Get processes in this queue from the queue API data
                // Only include processes with Ready or Running status
                const queueProcesses = queue.processes
                    .map(pid => processMap[pid])
                    .filter(process => process && (process.status === 'Ready' || process.status === 'Running'));

                const processItems = queueProcesses.length > 0
                    ? queueProcesses.map(p => `<span class="process-item ${p.pid === processesData.currentRunningPid ? 'running-process' : ''}">P${p.pid}</span>`).join('')
                    : '<span class="empty-queue">Empty</span>';

                queueDiv.innerHTML = `
                    <div class="queue-header">
                        <span class="queue-name">Queue ${index} ${index === 0 ? '(Highest Priority)' : index === queuesData.queues.length - 1 ? '(Lowest Priority)' : ''}</span>
                        <span class="queue-quantum">Quantum: ${queue.quantum}</span>
                    </div>
                    <div class="process-list">
                        ${processItems}
                    </div>
                `;
                container.appendChild(queueDiv);
            });
        } catch (e) {
            console.log('Failed to update queues');
        }
    }

    startSimulation() {
        if (!this.isRunning) {
            this.isRunning = true;
            this.intervalId = setInterval(() => this.stepSimulation(), 500);
            document.getElementById('start-btn').disabled = true;
            document.getElementById('pause-btn').disabled = false;
        }
    }

    pauseSimulation() {
        if (this.isRunning) {
            this.isRunning = false;
            clearInterval(this.intervalId);
            document.getElementById('start-btn').disabled = false;
            document.getElementById('pause-btn').disabled = true;
        }
    }

    async stepSimulation() {
        try {
            await fetch('/api/step', { method: 'POST' });
            this.updateDisplay();
        } catch (e) {
            console.log('Step failed');
        }
    }

    async resetSimulation() {
        this.pauseSimulation();
        try {
            await fetch('/api/reset', { method: 'POST' });
            // Wait briefly to ensure the reset is processed on the backend
            await new Promise(resolve => setTimeout(resolve, 100));

            // After reset, update display to reflect the empty state
            await this.updateDisplay();
        } catch (e) {
            console.log('Reset failed');
        }
    }

    showPreset() {
        document.getElementById('preset-modal').style.display = 'block';
    }

    showRandom() {
        document.getElementById('random-modal').style.display = 'block';
    }

    showConfig() {
        document.getElementById('config-modal').style.display = 'block';
    }

    showAddProcess() {
        document.getElementById('add-process-modal').style.display = 'block';
    }


    async loadPresetSet(setNumber) {
        try {
            // Temporarily stop auto-updates to prevent button state conflicts
            this.stopAutoUpdate();

            // Reset the scheduler first
            await fetch('/api/reset', { method: 'POST' });

            // Load the selected preset via API
            await fetch('/api/preset', {
                method: 'POST',
                headers:
                {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `set=${setNumber}`
            });

            // Re-enable buttons after loading processes
            document.getElementById('step-btn').disabled = false;
            document.getElementById('start-btn').disabled = false;

            // Update display once to show new processes
            await this.updateDisplay();

            // Resume auto-updates
            this.startAutoUpdate();

            this.closePreset();
        } catch (e) {
            console.log('Load preset failed');
            // Make sure to restart auto-updates even if there's an error
            this.startAutoUpdate();
        }
    }

    async confirmRandom() {
        try {
            // Temporarily stop auto-updates to prevent button state conflicts
            this.stopAutoUpdate();

            const count = document.getElementById('random-count').value;
            const minArrival = document.getElementById('random-min-arrival').value;
            const maxArrival = document.getElementById('random-max-arrival').value;
            const minBurst = document.getElementById('random-min-burst').value;
            const maxBurst = document.getElementById('random-max-burst').value;

            await fetch('/api/random',
            {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-form-urlencoded' },
                body: `count=${count}&minArrival=${minArrival}&maxArrival=${maxArrival}&minBurst=${minBurst}&maxBurst=${maxBurst}`
            });

            // Re-enable buttons after loading processes
            document.getElementById('step-btn').disabled = false;
            document.getElementById('start-btn').disabled = false;

            // Update display once to show new processes
            await this.updateDisplay();

            // Resume auto-updates
            this.startAutoUpdate();

            this.closeRandom();
        } catch (e) {
            console.log('Load random failed');
            // Make sure to restart auto-updates even if there's an error
            this.startAutoUpdate();
        }
    }

    async addNewProcess() {
        const arrival = document.getElementById('new-arrival-time').value;
        const burst = document.getElementById('new-burst-time').value;

        try {
            // Temporarily stop auto-updates to prevent button state conflicts
            this.stopAutoUpdate();

            await fetch('/api/add-process', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `arrival=${arrival}&burst=${burst}`
            });

            // Re-enable buttons after adding process
            document.getElementById('step-btn').disabled = false;
            document.getElementById('start-btn').disabled = false;

            // Update display once to show new process
            await this.updateDisplay();

            // Resume auto-updates
            this.startAutoUpdate();

            this.closeAddProcess();
        } catch (e) {
            console.log('Add process failed');
            // Make sure to restart auto-updates even if there's an error
            this.startAutoUpdate();
        }
    }

    closeConfig() {
        document.getElementById('config-modal').style.display = 'none';
    }

    closeAddProcess() {
        document.getElementById('add-process-modal').style.display = 'none';
        document.getElementById('new-arrival-time').value = '0';
        document.getElementById('new-burst-time').value = '5';
        document.getElementById('new-priority').value = '1';
    }

    closePreset() {
        document.getElementById('preset-modal').style.display = 'none';
    }

    closeRandom() {
        document.getElementById('random-modal').style.display = 'none';
    }

    async applyConfig() {
        const speed = document.getElementById('speed-slider').value;
        const algorithm = document.getElementById('last-queue-algorithm').value;
        const boost = document.getElementById('priority-boost').checked;
        const interval = document.getElementById('boost-interval').value;
        const numQueues = document.getElementById('num-queues').value;
        const baseQuantum = document.getElementById('base-quantum').value;
        const quantumMultiplier = document.getElementById('quantum-multiplier').value;

        try {
            await fetch('/api/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `algorithm=${algorithm}&boost=${boost}&interval=${interval}&speed=${speed}&numQueues=${numQueues}&baseQuantum=${baseQuantum}&quantumMultiplier=${quantumMultiplier}`
            });
            console.log('Config applied:', {
                speed, algorithm, boost, interval,
                numQueues, baseQuantum, quantumMultiplier
            });
        } catch (e) {
            console.log('Config update failed');
        }

        this.closeConfig();
    }
}

// Global functions for HTML onclick handlers
function applyConfig() {
    window.mlfqInterface.applyConfig();
}

function closeConfig() {
    window.mlfqInterface.closeConfig();
}

function addNewProcess() {
    window.mlfqInterface.addNewProcess();
}

function closeAddProcess() {
    window.mlfqInterface.closeAddProcess();
}

function selectPreset(setNumber) {
    window.mlfqInterface.loadPresetSet(setNumber);
}

function closePreset() {
    window.mlfqInterface.closePreset();
}

function confirmRandom() {
    window.mlfqInterface.confirmRandom();
}

function closeRandom() {
    window.mlfqInterface.closeRandom();
}

// Initialize when page loads
window.addEventListener('DOMContentLoaded', () => {
    window.mlfqInterface = new MLFQWebInterface();
});
