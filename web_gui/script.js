class MLFQWebInterface {
    constructor() {
        this.isRunning = false;
        this.intervalId = null;
        this.initializeEventListeners();
        this.updateDisplay();
        setInterval(() => this.updateDisplay(), 500);
    }

    initializeEventListeners() {
        document.getElementById('start-btn').onclick = () => this.startSimulation();
        document.getElementById('pause-btn').onclick = () => this.pauseSimulation();
        document.getElementById('step-btn').onclick = () => this.stepSimulation();
        document.getElementById('reset-btn').onclick = () => this.resetSimulation();
        document.getElementById('preset-btn').onclick = () => this.showPreset();
        document.getElementById('random-btn').onclick = () => this.showRandom();
        document.getElementById('config-btn').onclick = () => this.showConfig();
        document.getElementById('add-process-btn').onclick = () => this.showAddProcess();
        
        // Close modals when clicking outside
        document.getElementById('config-modal').onclick = (e) => {
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
        document.addEventListener('keydown', (e) => {
            if (e.key === 'Escape') {
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
            if (statusData.boostInterval) {
                document.getElementById('boost-timestamp').textContent = `Last boost: Time ${statusData.lastBoostTime}`;
                document.getElementById('boost-countdown').textContent = `Next boost in: ${statusData.nextBoostIn} time units`;
            }
            
            // Auto-stop when no processes left
            if (!statusData.hasProcesses && this.isRunning) {
                this.pauseSimulation();
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
                row.innerHTML = `
                    <td>P${process.pid}</td>
                    <td>${process.arrival}</td>
                    <td>${process.burst}</td>
                    <td>${process.remaining}</td>
                    <td>Q${process.queue}</td>
                    <td><span class="status-${process.status.toLowerCase()}">${process.status}</span></td>
                `;
            });
        } catch (e) {
            console.log('Failed to update process table');
        }
    }

    async updateQueues() {
        try {
            const response = await fetch('/api/queues');
            const data = await response.json();
            
            // Get process details
            const processResponse = await fetch('/api/processes');
            const processData = await processResponse.json();
            
            const container = document.getElementById('queues-container');
            container.innerHTML = '';
            
            data.queues.forEach((queue, index) => {
                const queueDiv = document.createElement('div');
                queueDiv.className = 'queue';
                
                // Get processes in this queue from the API data
                const queueProcesses = processData.processes.filter(p => 
                    p.queue === index && (p.status === 'Ready' || p.status === 'Running')
                );
                
                const processItems = queueProcesses.length > 0 
                    ? queueProcesses.map(p => `<span class="process-item">P${p.pid}</span>`).join('')
                    : '<span class="empty-queue">Empty</span>';
                
                queueDiv.innerHTML = `
                    <div class="queue-header">
                        <span class="queue-name">Queue ${index} ${index === 0 ? '(Highest Priority)' : index === data.queues.length - 1 ? '(Lowest Priority)' : ''}</span>
                        <span class="queue-quantum">Quantum: ${Math.pow(2, index)}</span>
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
            this.updateDisplay();
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

    async confirmPreset() {
        try {
            await fetch('/api/preset', { method: 'POST' });
            this.closePreset();
            this.updateDisplay();
        } catch (e) {
            console.log('Load preset failed');
        }
    }

    async loadPresetSet(setNumber) {
        try {
            // Reset the scheduler first
            await fetch('/api/reset', { method: 'POST' });
            
            // Load the selected preset via API
            const response = await fetch('/api/preset', { 
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `set=${setNumber}`
            });
            
            this.closePreset();
            this.updateDisplay();
        } catch (e) {
            console.log('Load preset failed');
        }
    }

    async confirmRandom() {
        try {
            await fetch('/api/random', { method: 'POST' });
            this.closeRandom();
            this.updateDisplay();
        } catch (e) {
            console.log('Load random failed');
        }
    }

    async addNewProcess() {
        const arrival = document.getElementById('new-arrival-time').value;
        const burst = document.getElementById('new-burst-time').value;
        
        try {
            await fetch('/api/add-process', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `arrival=${arrival}&burst=${burst}`
            });
            this.closeAddProcess();
            this.updateDisplay();
        } catch (e) {
            console.log('Add process failed');
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

    applyConfig() {
        const speed = document.getElementById('speed-slider').value;
        const algorithm = document.getElementById('last-queue-algorithm').value;
        const boost = document.getElementById('priority-boost').checked;
        const interval = document.getElementById('boost-interval').value;
        
        console.log('Config applied:', { speed, algorithm, boost, interval });
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

function confirmPreset() {
    window.mlfqInterface.confirmPreset();
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
