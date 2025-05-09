# Scheduling Simulator

A **CPU scheduling simulator** for experimenting with classic scheduling algorithms like First come first served, Round-Robin, Multi-Level Feedback Queue (MLFQ) etc... This website allows students and software engineers to visualize how processes are scheduled in an operating system using different strategies.

It combines a web-based frontend (HTML/CSS/JavaScript) with a C-based backend that implements the scheduling logic, and backend (Node.js) for server operations, giving both an educational and practical perspective.

---

## 🚀 Features

- **Multiple Scheduling Algorithms**: SJF, Priority-based, Stride, Lottery, FIFO, Round-Robin, MLFQ, and their semaphore-based variants implemented in C.
- **Web Interface**: User-friendly interface to input process data and visualize scheduling results.
- **Modular Codebase**: Clear separation between backend (business logic) and frontend (UI).
- **Cross-language Integration**: Uses Node.js to bridge C backend with a web frontend.
- **Output Logging**: Results are logged to a file for later inspection.

---

## 📁 Project Structure

```
├── Backend/
│ └── Business logic/
│ ├── src/
│ │ ├── round_robin.c/.h
│ │ ├── mlfq.c/.h
│ │ ├── pqueue.c/.h
│ │ ├── data_processor.c/.h
│ │ ├── utilities.c/.h
│ │ ├── schedulers.c/.h
│ │ ├── cJSON.c/.h
│ │ └── main.c
│ ├── Makefile
│ ├── app.js
│ ├── logfile.txt
│ ├── package.json
│ └── package-lock.json
├── Frontend/
│ ├── index.html
│ ├── index.js
│ ├── styles.css
│ ├── package.json
│ └── package-lock.json
└── README.md
```


## 🛠️ Technologies Used

- **C** – Core scheduling algorithms and backend logic (~89% of code).
- **JavaScript (Node.js)** – Handles backend-server and frontend interaction.
- **HTML/CSS/JS** – Frontend UI.
- **cJSON** – Lightweight C library for JSON parsing.
- **Make, GCC** – Build system for compiling the C backend.

---

## ⚙️ Installation

### Prerequisites

- **GCC and Make**
- **Node.js and npm**

### Setup

```bash
# Clone the repository
git clone https://github.com/ShadiBahaa/Scheduling-simulator.git
cd Scheduling-simulator

# Backend setup
cd Backend/Business\ logic
npm install        # install Node.js backend dependencies
make               # compile the C source files (creates ./bin/main)

# Frontend setup
cd ../../Frontend
npm install        # install frontend dependencies

```

## ▶️ Running the Simulator
### 1. Start Backend Server
```bash
cd Backend/Business\ logic
node app.js
```
This runs the Node.js backend, which communicates with the C scheduler.
### 2. Launch the Frontend
    Option 1: Navigate to http://localhost:3000 if app.js serves static files.
    Option 2: Open Frontend/index.html in your browser manually.
### 3. Run a Simulation
- Enter process details: arrival time, burst time, etc.

- Select a scheduling algorithm (e.g., Round-Robin or MLFQ).

- Click submit to see execution results and computed metrics (waiting time, turnaround time, Gantt chart, etc.).

Logs are saved in logfile.txt in the backend directory.
