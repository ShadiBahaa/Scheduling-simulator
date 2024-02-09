////////////////////////////////////////// Global Variables ///////////////////////////////////////////////

// Simulation Variables
// Set by user and sent to the server
let numCores = 1;
let animationSpeed = document.getElementById("speedRange").value;
let schedulingPolicyArray = []; // remember to update this when adding a new policy
const processes = [];
let processessArray = [];
let usingPriority = false;



// Set by the server
let dataArrays = [];
let analyticalData = null;

// Set during simulation
let timeArray = [];
let globalTime = null;
let intervalId = null;
let svgArray = [];
let intervalIdArray = [];
let runningAnimationArray = [];
let simulationStarted = false;

// SVG Variables
const barWidth = 30;
const barSpacing = 0;
let maxArrayLength = 0;
let svgWidth = 0;
const svgHeight = 115;
let colorScale = null;


////////////////////////////////////////// Binders //////////////////////////////////////////////
//Binders for Simulation
document.getElementById("startSimulatorBtn").addEventListener("click", startSimulation);
document.getElementById("resetBtn").addEventListener("click", resetAnimationForAllCores);
document.getElementById("pauseBtn").addEventListener("click", pauseAnimationForAllCores);
document.getElementById("resumeBtn").addEventListener("click", resumeAnimationForAllCores);
document.getElementById("stepForwardBtn").addEventListener("click", stepForwardForAllCores);
document.getElementById("stepBackwardBtn").addEventListener("click", stepBackwardForAllCores);
document.getElementById("speedRange").addEventListener("input", handleSpeedInputChange);
document.getElementById('backToSettings').addEventListener('click', goToSettings);


// Binders for Settings
document.getElementById('addCPUBurstBtn').addEventListener('click', () => addBurst('CPU'));
document.getElementById('addIOBurstBtn').addEventListener('click', () => addBurst('I/O'));
document.getElementById('clearBurstsBtn').addEventListener('click', clearBurstInputs);
document.getElementById('addProcessBtn').addEventListener('click', addProcess);
document.getElementById('goToSimulationBtn').addEventListener('click', goToSimulation);
document.getElementById('clearProcessesBtn').addEventListener('click', clearProcessList);
document.getElementById('schedulingPolicy').addEventListener('change', updateSchedulingPolicyInfo);
document.getElementById('schedulingPolicy').addEventListener('change', handleSchedulingPolicyChange);


////////////////////////////////////////// Server Functions //////////////////////////////////////////
const API_URL = 'http://localhost:3000/api';
let loading = false;

// make the payload of the request for starting the simulation
function makePayload() {

    const payload = {
        numCores: numCores,
        schedulingPolicy: schedulingPolicyArray[0],
        processes: processes
    }
    return payload;
}

// make a post request to the server
function simulate(payload) {
    loading = true;
    const url = `${API_URL}/simulate`;
    const options = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(payload)
    }
    fetch(url, options)
        .then(res => {
            if (res.status !== 200) {
                document.getElementById('loadingPlaceHolder').innerHTML = '<div class="process-letter" style="text-align: center;"><p>Simulation failed please try again.</p></div>';
                return;
            }
            return res.json();
        })
        .then(data => {
            dataArrays = data.schedulingOrder;
            dataArrays = processDataArrays(dataArrays);
            analyticalData = {
                averageTurnaroundTime: data.avgTurnaroundTime,
                averageResponseTime: data.avgResponseTime,
                totalIdleTime: 0,
                totalExecutionTime: 0,
                processesCount: data.processesCount,
                processes: data.processesAnalysis
            }
        }).then(() => {

            maxArrayLength = Math.max(...dataArrays.map((item) => item.length));
            svgWidth = (barWidth + barSpacing) * maxArrayLength + 25;
            timeArray = new Array(numCores).fill(-1);
            svgArray = new Array(numCores);
            intervalIdArray = new Array(numCores);
            runningAnimationArray = new Array(numCores).fill(false);
            loading = false;
            document.getElementById('loadingPlaceHolder').innerHTML = '<div class="process-letter" style="text-align: center;"><p>Simulation is ready</p></div>';

        }).catch(err => console.log(err));
}

// process the dataArrays recived from the server
function processDataArrays(dataArrays) {
    for (let i = 0; i < dataArrays.length; i++) {
        const dataArray = dataArrays[i];
        const processedDataArray = [];
        for (let j = 0; j < dataArray.length; j++) {
            const process = dataArray[j];
            const timeSlot = j + 1;
            const processedData = {
                name: process,
                timeSlot: timeSlot
            }
            processedDataArray.push(processedData);
        }
        // add last element to the array to make the animation stop at the end
        const lastElement = processedDataArray[processedDataArray.length - 1];
        const lastElementCopy = { ...lastElement };
        lastElementCopy.timeSlot = lastElementCopy.timeSlot + 1;
        lastElementCopy.name = 'e&';
        processedDataArray.push(lastElementCopy);
        dataArrays[i] = processedDataArray;
    }
    return dataArrays;
}


////////////////////////////////////////// Simulation Functions //////////////////////////////////////////

function startSimulation() {
    if (loading) {
        return;
    } else {
        document.getElementById('loadingPlaceHolder').style.display = 'none';
        document.getElementById('processChart').style.display = 'block';
    }

    if (runningAnimationArray.includes(true) || simulationStarted) {
        pauseAnimationForAllCores();
        simulationStarted = false;
        for (let i = 0; i < numCores; i++) {
            timeArray[i] = -1;
            if (i === 0) {
                globalTime = -2;
                updateRunningTime();
            }
        }
    }

    processessArray = [...analyticalData.processes.map(p => p.id)];
    colorScale = d3.scaleOrdinal().domain(processessArray).range(d3.schemeCategory10);

    numCores = document.getElementById('numCores').value;
    const svgContainer = document.getElementById('processChart');
    svgContainer.innerHTML = '';

    const processBoxes = document.createElement('div');
    processBoxes.className = 'process-boxes';
    // add a global time box
    const globalTimeBox = document.createElement('div');
    globalTimeBox.className = 'global-time-box';
    globalTimeBox.innerHTML = `
        <div class="global-time-box-item">
            <p><strong>Global Time: <span id="globalTime">0</span></strong> </p>
        </div>
    `;
    processBoxes.appendChild(globalTimeBox);
    processessArray.forEach(process => {
        const processBox = document.createElement('div');
        processBox.className = 'process-box';
        // if using priority change the height of the process boxes
        processBox.style.height = usingPriority ? '165px' : '145px';
        // create a paragraph for the process name
        const processName = document.createElement('p');
        if (process === 'i') {
            processName.textContent = 'Idle';
            processBox.classList.add('idle');
            // add the analysis of the process
            const processAnalysisDiv = document.createElement('div');
            processAnalysisDiv.classList.add('process-analysis');
            processAnalysisDiv.innerHTML = `
                <div class="process-analysis-item">
                    <p><strong>Total Idle Time: ${analyticalData.totalIdleTime}</strong> </p>
                </div>
            `;
            processBox.appendChild(processName);
            processBox.appendChild(processAnalysisDiv);
        } else {
            processName.textContent = `Process ${process}`;
            // add the analysis of the process
            const processAnalysis = analyticalData.processes.find(p => p.id === process);
            const processAnalysisDiv = document.createElement('div');
            processAnalysisDiv.classList.add('process-analysis');
            processAnalysisDiv.innerHTML = `
                <div class="process-analysis-item" style="display:${usingPriority ? 'block' : 'none'};">
                    <p><strong>Priority: ${processes.find(p => p.id === process).priority} </strong> </p>
                </div>            
                <div class="process-analysis-item">
                    <p><strong>Arrival Time: ${processAnalysis.arrivalTime} </strong> </p>
                </div>
                <div class="process-analysis-item">
                    <p><strong>First Run Time: ${processAnalysis.firstRunTime} </strong> </p>
                </div>
                <div class="process-analysis-item">
                    <p><strong>Response Time: ${processAnalysis.responseTime} </strong> </p>
                </div>
                <div class="process-analysis-item">
                    <p><strong>Turnaround Time: ${processAnalysis.turnaroundTime} </strong> </p>
                </div>
                <div class="process-analysis-item">
                    <p><strong>Status: <span id=${process}>Has not arrived</span></strong> </p>
                </div>
            `;
            processBox.appendChild(processName);
            processBox.appendChild(processAnalysisDiv);
        }
        processBox.style.backgroundColor = colorScale(process);
        processBoxes.appendChild(processBox);
    });

    // add a process box for system analysis
    const processBox = document.createElement('div');
    processBox.className = 'process-box';

    // create a paragraph for the process name
    const processName = document.createElement('p');
    processName.textContent = `System Analysis`;
    processBox.appendChild(processName);
    // add the analysis of the process
    const processAnalysisDiv = document.createElement('div');
    processAnalysisDiv.classList.add('process-analysis');
    processAnalysisDiv.innerHTML = `
        <div class="process-analysis-item">
            <p><strong>Avg. Turnaround Time: ${analyticalData.averageTurnaroundTime.toFixed(2)} </strong> </p>
        </div>
        <div class="process-analysis-item">
            <p><strong>Avg. Response Time: ${analyticalData.averageResponseTime.toFixed(2)} </strong> </p>
        </div>
    `;
    processBox.appendChild(processAnalysisDiv);
    processBox.style.backgroundColor = colorScale(`System Analysis`);
    processBoxes.appendChild(processBox);


    svgContainer.appendChild(processBoxes);

    for (let i = 0; i < numCores; i++) {
        const divId = `div_${i}`;
        const div = document.createElement('div');
        div.id = divId;
        div.className = 'svg-container';

        const coreInformation = document.createElement('div');
        coreInformation.className = 'core-information';
        coreInformation.textContent = `Core ${i + 1} - ${schedulingPolicyArray[0]}`;

        svgContainer.appendChild(coreInformation);
        svgContainer.appendChild(div);

        // make that svg scrollable
        div.style.width = `${svgWidth + 10}px`;
        div.style.overflowX = 'scroll';

        const svg = d3
            .select(`#${divId}`)
            .append("svg")
            .attr("width", svgWidth)
            .attr("height", svgHeight);

        svgArray[i] = svg;
        timeArray[i] = -1;
        if (i === 0) {
            globalTime = -2;
            updateRunningTime();
        }
        startAnimation(svg, dataArrays[i], i);
    }
    simulationStarted = true;
}


function startAnimation(svg, processData, coreNumber) {
    if (runningAnimationArray[coreNumber]) {
        return; // Return if animation is already running
    }
    runningAnimationArray[coreNumber] = true;
    intervalIdArray[coreNumber] = setInterval(function () {
        if (timeArray[coreNumber] === processData.length) {
            clearInterval(intervalIdArray[coreNumber]);
            runningAnimationArray[coreNumber] = false;
            return;
        }
        timeArray[coreNumber]++;
        if (coreNumber === 0) {
            globalTime++;
            updateRunningTime();
        }
        const processedDataSlice = processData.slice(0, timeArray[coreNumber]);
        renderChart(processedDataSlice, svg, dataArrays[coreNumber]);
    }, animationSpeed);
}

// function to update the global time and the status of each process
function updateRunningTime() {
    // update the status of each process
    document.getElementById('globalTime').textContent = globalTime < 0 ? 'Not started' : globalTime;
    for (let i = 0; i < analyticalData.processesCount; i++) {
        const process = analyticalData.processes[i];
        if (process.arrivalTime > globalTime) {
            document.getElementById(process.id).textContent = 'Has not arrived';
        } else if (globalTime - process.arrivalTime >= process.statusCapacity) {
            document.getElementById(process.id).textContent = 'Finished';
        } else {
            switch (process.status[globalTime - process.arrivalTime]) {
                case 1:
                    document.getElementById(process.id).textContent = 'Running';
                    break;
                case 0:
                    document.getElementById(process.id).textContent = 'Ready';
                    break;
                case -1:
                    document.getElementById(process.id).textContent = 'Blocked';
                    break;
                default:
                    break;
            }
        }
    }
}

// function to render the chart
function renderChart(processedData, svg, dataToBeProcessed) {
    const bars = svg.selectAll("rect.bar").data(processedData, (d) => d.timeSlot);
    const barsEnter = bars
        .enter()
        .append("rect")
        .attr("class", "bar")
        .attr("x", (d, index) => index * (barWidth + barSpacing))
        .attr("y", 0) // Set the initial position for entering bars
        .attr("width", barWidth)
        .attr("height", 50)
        .attr("fill", (d) => colorScale(d.name))
        .attr("stroke", "black")
        .attr("stroke-width", 1.5);

    const barsUpdate = bars.merge(barsEnter);
    barsUpdate.transition().duration(250).attr("y", 45); // Set the final position for exiting bars
    bars
        .exit()
        .transition()
        .duration(250)
        .attr("y", 0)
        .remove();

    const textLabels = svg
        .selectAll("text.processName")
        .data(processedData, (d) => d.timeSlot);
    const textLabelsEnter = textLabels
        .enter()
        .append("text")
        .attr("class", "processName")
        .attr("x", (d, index) => index * (barWidth + barSpacing) + barWidth / 2)
        .attr("y", 36.5) // Set the initial position for entering text
        .attr("text-anchor", "middle")
        .attr("fill", "white");
    textLabelsEnter
        .merge(textLabels)
        .transition()
        .duration(250)
        .attr("y", 76.5); // Set the final position for exiting text

    textLabelsEnter.merge(textLabels).text((d) => d.name);
    textLabels.exit().transition().duration(250).attr("y", 30).remove();

    const xAxisScale = d3
        .scaleLinear()
        .domain([0, dataToBeProcessed.length])
        .range([0, (barWidth + barSpacing) * dataToBeProcessed.length]);
    const xAxis = d3.axisBottom(xAxisScale).ticks(dataToBeProcessed.length);

    svg.select(".x-axis").remove(); // Remove existing axis before appending new one
    svg
        .append("g")
        .attr("class", "x-axis")
        .attr("transform", "translate(0, 95)")
        .call(xAxis);
}


function clearSVG() {
    clearInterval(intervalId);
    svg.selectAll("*").remove();
}

function resetAnimationForAllCores() {
    if (!simulationStarted) {
        return;
    }
    pauseAnimationForAllCores();
    for (let i = 0; i < numCores; i++) {
        timeArray[i] = -1;
        globalTime = -2;
        const svg = svgArray[i];
        clearInterval(intervalIdArray[i]);
        startAnimation(svg, dataArrays[i], i);
    }
}

function pauseAnimationForAllCores() {
    if (!simulationStarted) {
        return;
    }
    for (let i = 0; i < numCores; i++) {
        clearInterval(intervalIdArray[i]);
    }
    runningAnimationArray.fill(false);
}

function resumeAnimationForAllCores() {
    if (!simulationStarted) {
        return;
    }
    for (let i = 0; i < numCores; i++) {
        const svg = svgArray[i];
        startAnimation(svg, dataArrays[i], i);
    }
}

function stepForwardForAllCores() {
    if (!simulationStarted) {
        return;
    }
    if (runningAnimationArray.includes(true)) {
        return;
    }
    for (let i = 0; i < numCores; i++) {
        if (timeArray[i] === dataArrays[i].length) {
            continue;
        }
        timeArray[i]++;
        if (i === 0) {
            globalTime++;
            updateRunningTime();
        }
        const svg = svgArray[i];
        const processedDataSlice = dataArrays[i].slice(0, timeArray[i]);
        renderChart(processedDataSlice, svg, dataArrays[i]);
    }
}

function stepBackwardForAllCores() {
    if (!simulationStarted) {
        return;
    }
    if (runningAnimationArray.includes(true)) {
        return;
    }
    let maxTime = Math.max(...timeArray);
    for (let i = 0; i < numCores; i++) {
        if (timeArray[i] === -1 || timeArray[i] === 0) {
            continue;
        }
        if (maxTime > timeArray[i]) {
            continue;
        }
        timeArray[i]--;
        if (i === 0) {
            globalTime--;
            updateRunningTime();
        }
        const svg = svgArray[i];
        const processedDataSlice = dataArrays[i].slice(0, timeArray[i]);
        renderChart(processedDataSlice, svg, dataArrays[i]);
    }
}

function handleSpeedInputChange(event) {
    animationSpeed = event.target.value;
    document.getElementById("speedValue").innerHTML = animationSpeed;

    if (simulationStarted) {
        resetAnimationForAllCores();
    }
}


document.getElementById("speedValue").innerHTML = document.getElementById("speedRange").value;



let tempProcessObject = {
    id: '',
    arrivalTime: 0,
    totalDuration: 0,
    priority: -1,
    bursts: [],
};


////////////////////////////////////////// Settings Functions ////////////////////////////////////////////

function calculateSumOfBurstDurations(bursts) {
    return bursts.reduce((sum, burst) => sum + burst.duration, 0);
}

function createBurstElement(burst) {
    const burstDiv = document.createElement('div');
    burstDiv.classList.add('burst-input');
    burstDiv.textContent = `${burst.type}: ${burst.duration}`;
    return burstDiv;
}

function createArrowElement() {
    const arrow = document.createElement('div');
    arrow.classList.add('arrow');
    arrow.textContent = '>>';
    return arrow;
}

function addBurst(type) {
    const burstInputs = document.getElementById('burstInputs');
    const burstDuration = document.getElementById('burstDuration').value;

    // Use the displayErrorMessage function to check burst duration validity
    if (parseInt(burstDuration) <= 0) {
        displayErrorMessage('The burst duration must be greater than 0');
        return;
    }
    // Check if the burst type is the same as the previous burst type
    if (tempProcessObject.bursts.length > 0) {
        if (tempProcessObject.bursts[tempProcessObject.bursts.length - 1].type === type) {
            displayErrorMessage('The burst type must be different from the previous burst type');
            return;
        }
    }
    // Check if the first burst is an I/O burst
    if (tempProcessObject.bursts.length === 0 && type === 'I/O') {
        displayErrorMessage('The first burst must be a CPU burst');
        return;
    }
    // Create burst element
    const burstElement = createBurstElement({ type, duration: parseInt(burstDuration) });
    // Create arrow element
    const arrow = createArrowElement();
    // Append arrow if needed
    if (burstInputs.children.length > 0) {
        burstInputs.appendChild(arrow);
    }
    // Append burst element
    burstInputs.appendChild(burstElement);
    // Update tempProcessObject
    tempProcessObject.bursts.push({ type, duration: parseInt(burstDuration) });
    // add the burst duration to the total duration
    tempProcessObject.totalDuration += parseInt(burstDuration);
    // update the burst duration input with current total duration
    document.getElementById('totalDuration').value = tempProcessObject.totalDuration;
}


function clearBurstInputs() {
    const burstInputs = document.getElementById('burstInputs');
    burstInputs.innerHTML = '';
    tempProcessObject = {
        id: '',
        arrivalTime: 0,
        totalDuration: 0,
        priority: -1,
        bursts: [],
    };
    document.getElementById('totalDuration').value = 0;
}

function addProcess() {
    const arrivalTime = parseInt(document.getElementById('arrivalTime').value);
    const totalDuration = parseInt(document.getElementById('totalDuration').value);
    const priority = parseInt(document.getElementById('priority').value);
    const bursts = tempProcessObject.bursts;

    // Check if arrival time is valid
    if (arrivalTime < 0) {
        displayErrorMessage('The arrival time must be greater than or equal to 0');
        return;
    }
    // Check if total duration is valid
    if (totalDuration <= 0) {
        displayErrorMessage('The total duration must be greater than 0');
        return;
    }
    // Check if pirority is valid
    if (usingPriority && (priority < 0 || priority > 100)) {
        displayErrorMessage('The priority must be between 0 and 100');
        return;
    }
    // Check if total duration is equal to the sum of burst durations
    if (calculateSumOfBurstDurations(bursts) !== totalDuration) {
        displayErrorMessage('The total duration must be equal to the sum of the burst durations');
        return;
    }
    // Create process object
    tempProcessObject.id = document.getElementById('processLetter').value;
    tempProcessObject.arrivalTime = arrivalTime;
    tempProcessObject.totalDuration = totalDuration;
    tempProcessObject.priority = parseInt(document.getElementById('priority').value);


    // Display process information
    const processListDiv = document.getElementById('processList');
    const processDiv = document.createElement('div');
    processDiv.classList.add('process');

    const processLetter = document.createElement('div');
    processLetter.classList.add('process-letter');
    processLetter.textContent = `Process ${tempProcessObject.id}`;

    const processPriority = document.createElement('div');
    processPriority.classList.add('process-priority');
    processPriority.textContent = `Priority: ${tempProcessObject.priority}`;

    const processArrivalTime = document.createElement('div');
    processArrivalTime.classList.add('process-arrival-time');
    processArrivalTime.textContent = `Arrival Time: ${tempProcessObject.arrivalTime}`;

    const processTotalDuration = document.createElement('div');
    processTotalDuration.classList.add('process-total-duration');
    processTotalDuration.textContent = `Total Duration: ${tempProcessObject.totalDuration}`;

    const processBursts = document.createElement('div');
    processBursts.innerText = 'Bursts:';
    processBursts.classList.add('process-bursts');

    bursts.forEach(burst => {
        const burstDiv = createBurstElement(burst);
        const arrow = createArrowElement();
        if (processBursts.children.length > 0) {
            processBursts.appendChild(arrow);
        }
        processBursts.appendChild(burstDiv);
    });

    // remove placeholder process text
    if (processListDiv.children.length === 0) {
        document.getElementById('processPlaceHolder').style.display = 'none';
    }

    // Add process information to the DOM
    processDiv.appendChild(processLetter);
    processDiv.appendChild(processPriority);
    processDiv.appendChild(processArrivalTime);
    processDiv.appendChild(processTotalDuration);
    processDiv.appendChild(processBursts);
    processListDiv.appendChild(processDiv);

    // Add process to the processes array
    processes.push(tempProcessObject);

    // Reset the tempProcessObject
    tempProcessObject = {
        id: '',
        arrivalTime: 0,
        totalDuration: 0,
        priority: -1,
        bursts: [],
    };

    // Remove the selected process letter from the dropdown
    const processLetterSelect = document.getElementById('processLetter');
    const selectedProcessLetterIndex = processLetterSelect.selectedIndex;
    processLetterSelect.remove(selectedProcessLetterIndex);
    if (processLetterSelect.length === 0) {
        document.getElementById('addProcessBtn').disabled = true;
    }

    // Reset the burst inputs
    const burstInputs = document.getElementById('burstInputs');
    burstInputs.innerHTML = '';
    resetInputs();
}

function clearProcessList() {
    const processListDiv = document.getElementById('processList');
    processListDiv.innerHTML = '';
    document.getElementById('processPlaceHolder').style.display = 'block';
    // Reset the tempProcessObject
    tempProcessObject = {
        id: '',
        arrivalTime: 0,
        totalDuration: 0,
        priority: -1,
        bursts: [],
    };

    // Reset the burst inputs
    const burstInputs = document.getElementById('burstInputs');
    burstInputs.innerHTML = '';
    resetInputs();

    // Reset the process letter select element
    const processLetterSelect = document.getElementById('processLetter');
    processLetterSelect.innerHTML = '';
    initializeLetters();

    // Reset the processes array
    processes.length = 0;
}

// update the info span when the user changes the scheduling policy
function updateSchedulingPolicyInfo() {
    // resetInputs();
    // clearBurstInputs();
    // clearProcessList();
    const infoSpan = document.getElementById('schedulingPolicyInfo');
    document.getElementById('priorityInfo').style.display = 'none';
    usingPriority = false;
    switch (schedulingPolicyArray[0]) {
        case 'FCFS1':
            infoSpan.textContent = '*First Come First Served and stays on the CPU until it finishes';
            break;
        case 'FCFS2':
            infoSpan.textContent = '*First Come First Served and leaves the CPU when it is blocked';
            break;
        case 'SJF1':
            infoSpan.textContent = '*Shortest Job First based on burst time and leaves the CPU when it is blocked';
            break;
        case 'SJF2':
            infoSpan.textContent = '*Shortest Job First based on total duration and doesn\'t leave the CPU when it is blocked';
            break;
        case 'SJF3':
            infoSpan.textContent = '*Preemptive Shortest Job First based on total duration and doesn\'t leave the CPU when it is blocked';
            break;
        case 'STCF1':
            infoSpan.textContent = '*Shortest Time to Completion First based on burst time and leaves the CPU when it is blocked';
            break;
        case 'Priority1':
            infoSpan.textContent = '*Non-preemptive Priority and leaves the CPU when it is blocked';
            document.getElementById('priorityInfo').style.display = 'block';
            usingPriority = true;
            break;
        case 'Priority2':
            infoSpan.textContent = '*Preemptive Priority and leaves the CPU when it is blocked';
            document.getElementById('priorityInfo').style.display = 'block';
            usingPriority = true;
            break;
        case 'Priority3':
            infoSpan.textContent = '*Non-Preemptive Priority and doesn\'t leave the CPU when it is blocked';
            document.getElementById('priorityInfo').style.display = 'block';
            usingPriority = true;
            break;
        case 'Priority4':
            infoSpan.textContent = '*Preemptive Priority and doesn\'t leave the CPU when it is blocked';
            document.getElementById('priorityInfo').style.display = 'block';
            usingPriority = true;
            break;
        case 'Stride':
            infoSpan.textContent = '*Stride Scheduling';
            document.getElementById('priorityInfo').style.display = 'block';
            usingPriority = true;
            break;
        case 'Lottery':
            infoSpan.textContent = '*Lottery Scheduling';
            document.getElementById('priorityInfo').style.display = 'block';
            usingPriority = true;
            break;
        case 'RR':
            infoSpan.textContent = '*Round Robin';
            break;
        case 'MLFQ':
            infoSpan.textContent = '*Multi Level Feedback Queue';
            break;
        default:
            break;
    }

}

// handle selecting the scheduling policy
function handleSchedulingPolicyChange() {
    const schedulingPolicy = document.getElementById('schedulingPolicy').value;
    schedulingPolicyArray[0] = schedulingPolicy;
    updateSchedulingPolicyInfo();
}



////////////////////////////////////////// Navigation Functions ////////////////////////////////////////////

function goToSettings() {
    // Stop the simulation if it is running
    if (runningAnimationArray.includes(true) || simulationStarted) {
        pauseAnimationForAllCores();
        simulationStarted = false;
        for (let i = 0; i < numCores; i++) {
            timeArray[i] = -1;
        }
    }
    document.getElementById('settings-container').style.display = 'block';
    document.getElementById('simulation-container').style.display = 'none';
    document.getElementById('processChart').innerHTML = '';
    document.getElementById('loadingPlaceHolder').style.display = 'block';
    document.getElementById('processChart').style.display = 'none';
    document.getElementById('loadingPlaceHolder').innerHTML = '<div class="process-letter" style="text-align: center;"><p>Preparing your simulation be patient</p></div>';
}

function goToSimulation() {
    const processListDiv = document.getElementById('processList');
    if (processListDiv.children.length === 0) {
        displayErrorMessage('You must add at least one process');
        return;
    }
    // get the schduling policy
    schedulingPolicyArray[0] = document.getElementById('schedulingPolicy').value;
    // get the number of cores
    numCores = document.getElementById('numCores').value;
    document.getElementById('settings-container').style.display = 'none';
    document.getElementById('simulation-container').style.display = 'block';

    // processessArray = [...processes.map(p => p.id), 'i'];
    // colorScale = d3.scaleOrdinal().domain(processessArray).range(d3.schemeCategory10);
    simulate(makePayload());
}


////////////////////////////////////////// Helper Functions //////////////////////////////////////////////

// Function to display an error message and hide it after a specified duration
function displayErrorMessage(message, duration = 3000) {
    const errorElement = document.getElementById('error');
    errorElement.textContent = message;
    errorElement.style.display = 'block';

    setTimeout(() => {
        errorElement.style.display = 'none';
    }, duration);
}

// reset all input values
const resetInputs = () => {
    document.getElementById('arrivalTime').value = 0;
    document.getElementById('totalDuration').value = 0;
    document.getElementById('burstDuration').value = 1;
    document.getElementById('burstInputs').innerHTML = '';
    document.getElementById('priority').value = 0;
};

// Initialize the process letter select element
function initializeLetters() {
    const processLetterSelect = document.getElementById('processLetter');

    // Array of capital letters
    const capitalLetters = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'];

    // Populate the select element with options
    capitalLetters.forEach(letter => {
        const option = document.createElement('option');
        option.value = letter;
        option.textContent = letter;
        processLetterSelect.appendChild(option);
    });
}


////////////////////////////////////////// Initialize //////////////////////////////////////////////

window.onload = () => {
    resetInputs();
    initializeLetters();
    handleSchedulingPolicyChange();

};