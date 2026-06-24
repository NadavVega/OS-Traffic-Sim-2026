# OS Project: Directed Graph Traffic Simulation - "The Schedulers"

## 1. Project Overview

A simulation of movement inside a directed weighted graph, exploring operating system concepts such as process creation, parent/child execution, IPC, synchronization, and scheduling.

The project models travelers moving through graph nodes and edges. Each traveler has a source and destination node. The shortest route is calculated with Dijkstra's algorithm, and the simulation is displayed with Raylib.

## 2. The Team

| Name         | Responsibilities                                                                                                                                                                                                                                                                                                                                                                    |
| :----------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Nadav**    | Created the communication system between child processes and the parent process using pipes. Implemented parent-side passive manager logic for Milestone 5, including non-blocking pipe reads, GUI updates from IPC messages, required parent logs, and child reaping. For the final milestone, responsible for final integration, README update, demo video, and final GitHub tag. |
| **Nave**     | Connected project parts, verified GUI behavior, helped preserve previous milestone behavior, and tested the submission against assignment requirements. For Milestone 7, responsible for showing the active scheduler in the GUI.                                                                                                                                                   |
| **Bar**      | Made the child process autonomous for Milestone 5. Implemented Dijkstra execution inside child process code, simulated child travel along the calculated path, and sent progress updates to the parent using IPC. For Milestone 7, responsible for scheduler IPC request and grant flow.                                                                                            |
| **Binyamin** | Responsible for the Milestone 7 scheduler core and node waiting queues, including FCFS/SJF queue logic and scheduler state management.                                                                                                                                                                                                                                              |

## 3. Implementation Description

- **Milestone 1**: Implemented Dijkstra's algorithm using an adjacency matrix. Handles directed edges and weight calculations.
- **Milestone 2**: Created a circular layout for graph nodes. Visualized directed edges with arrows and legible weights using Raylib.
- **Milestone 3**: Added a moving entity that travels along the calculated shortest path. Implemented a 1-second pause at nodes and 300ms movement jumps per edge weight unit.
- **Milestone 4**: Transitioned to a multi-process architecture. The parent process reads the input, calculates each traveler's path using Dijkstra, creates one child process per traveler using `fork()`, and controls GUI movement. Each child prints `[PID] started` and waits. When a traveler finishes its route, the parent sends a signal to terminate the matching child and then reaps it to prevent zombie processes.
- **Milestone 5**: Changed the architecture so that child processes are autonomous. The parent no longer calculates the route for each traveler. Instead, each child calculates its own Dijkstra path, simulates movement along the path, and reports progress to the parent using IPC. The parent acts as a passive manager: it reads IPC messages, prints required logs, updates the GUI according to child reports, and reaps child processes when they finish.
- **Milestone 6**: Added node synchronization. The parent creates one System V semaphore per graph node before forking children and destroys the semaphore set during cleanup. A child sends `IPC_WAITING_LOCK` before trying to enter a node, sends `IPC_INSIDE_NODE` after it gets the lock, stays inside the node for 1 second, releases the lock, and then sends its normal movement update. Entering and staying inside a node is the critical section.
- **Milestone 7**: Added scheduler support for node waiting order. The parent can run the simulation with either `fcfs` or `sjf`. The scheduler manages waiting queues per graph node and chooses which waiting traveler should enter next. FCFS selects the first waiting traveler in the node queue. SJF selects the waiting traveler with the shortest next edge weight, using that next travel time as the job length.

## 4. IPC, Synchronization, and Scheduling Design

### Milestone 5 IPC Design

For Milestone 5, we chose to use pipes.

Pipes fit this milestone because the communication is mainly child-to-parent: child processes send progress updates to the parent. Each child sends small fixed-size messages containing its PID, current node, next node, and status. This keeps the implementation simple and avoids the extra synchronization logic that shared memory would require.

The parent reads IPC messages in a non-blocking way so that the Raylib GUI loop does not freeze while waiting for child messages.

### Milestone 6 Synchronization Design

For Milestone 6, we chose to use System V semaphores.

Normal mutexes are not enough here because the travelers are separate processes, not threads in the same process. System V semaphores are process-safe, so all child processes can use the same node locks.

Each node semaphore starts at `1`, so only one traveler can be inside that node at a time. If another traveler reaches the same node while it is busy, that traveler waits outside until the node becomes available.

### Milestone 7 Scheduler Design

Milestone 7 adds parent-side scheduling for node waiting queues.

The scheduler module is responsible for:

- creating scheduler state
- maintaining a waiting queue per graph node
- adding a waiting traveler/process to a node queue
- choosing the next traveler for a node
- marking a node as busy/free
- destroying scheduler state during cleanup

Supported algorithms:

- **FCFS**: First Come First Served. The first traveler that requested entry to a node is selected first.
- **SJF**: Shortest Job First. The traveler with the shortest next edge weight is selected first. The next edge weight represents the next travel time / job length.

The scheduler does not replace the GUI and does not replace Dijkstra. It only controls the waiting order for travelers that want to enter a node.

## 5. Milestone 7 Work Plan / Team Split

| Team Member  | Milestone 7 Responsibility                                                                                                                                                    |
| :----------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Binyamin** | Scheduler core and node waiting queues. Creates the scheduler module and implements FCFS/SJF queue selection logic.                                                           |
| **Nave**     | Show active scheduler in the GUI. Displays whether the simulation is currently running with FCFS or SJF.                                                                      |
| **Bar**      | Scheduler IPC request and grant flow. Connects child waiting requests with parent-side scheduling decisions.                                                                  |
| **Nadav**    | Final integration, README, demo video, and final GitHub tag. Verifies that Milestone 7 is connected with the existing Milestone 6 behavior and prepares the final submission. |

## 6. How to Build & Run

### Clean Build Files

```bash
make clean
```

### Milestone 1

```bash
make milestone1
./dijkstra <file-name>
```

### Milestones 2-6

```bash
make milestone6
./sim <file-name>
```

### Milestone 7

Build:

```bash
make milestone7
```

Run with FCFS:

```bash
./sim -schd fcfs <file-name>
```

Run with SJF:

```bash
./sim -schd sjf <file-name>
```

The old command is still supported and defaults to FCFS:

```bash
./sim <file-name>
```

## 7. Milestone 7 Test Input

A scheduler comparison input file should be included at:

```bash
test1.txt
```

Recommended test behavior:

- Use 8-10 graph nodes.
- Use about 10 travelers.
- Create a shared bottleneck node.
- Make several travelers request the same node.
- Give the outgoing edges from the bottleneck different weights.
- FCFS should show request/input order.
- SJF should prefer the shortest next edge weight.

Final submission should include:

- all milestones 1-7
- working `Makefile`
- updated `README.md`
- Milestone 7 test input
- short demo video
- final GitHub tag
