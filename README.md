# OS Project: Directed Graph Traffic Simulation - "The Schedulers"

## 1. Project Overview

A simulation of movement within a directed graph, exploring OS concepts such as concurrency, IPC, and synchronization.

## 2. The Team

| Name         | Responsibilities                                                                                                                                                                                                                                                                                                                                                  |
| :----------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Nadav**    | Created the communication system between the child processes and the parent process using pipes. Also implemented the parent-side passive manager logic for Milestone 5, where the parent waits for messages from the children in a non-blocking way, updates the GUI according to those messages, prints the required logs, and reaps child processes correctly. |
| **Nave**     | Connected the different project parts, verified that the GUI works correctly, helped preserve the previous milestone behavior, and tested that the submission holds up to the assignment requirements.                                                                                                                                                            |
| **Bar**      | Made the child process fully autonomous for Milestone 5. Implemented Dijkstra execution inside the child process code, simulated the child “ride” along the calculated path, and sent progress updates to the parent using IPC.                                                                                                                                   |
| **Binyamin** | Was originally assigned to the parent passive-manager task, but was not present for this milestone due to active military service. This part was completed by Nadav.                                                                                                                                                                                              |

## 3. Implementation Description

- **Milestone 1**: Implemented Dijkstra's algorithm using an adjacency matrix. Handles directed edges and weight calculations.
- **Milestone 2**: Created a circular layout for nodes. Visualized edges with directed arrows and legible weights using Raylib.
- **Milestone 3**: Added a golden entity that moves along the calculated path. Implemented a 1-second pause at nodes and 300ms jumps per weight unit.
- **Milestone 4**: Transitioned to a multi-process architecture. The parent process reads the input, calculates each traveler’s path using Dijkstra, creates one child process per traveler using `fork()`, and controls the GUI movement. Each child prints `[PID] started` and then waits. When a traveler finishes its route, the parent sends a signal to terminate the matching child process and then reaps it to prevent zombie processes.
- **Milestone 5** :Changed the architecture so that child processes are autonomous. The parent no longer calculates the route for each traveler. Instead, each child process calculates its own Dijkstra path, simulates its movement along the path, and reports progress to the parent using IPC. The parent acts as a passive manager: it reads IPC messages, prints the required logs, updates the GUI according to child reports, and reaps child processes when they finish.
- **Milestone 6**: Added node synchronization. The parent creates one System V semaphore per graph node before forking the children, and destroys the semaphore set during cleanup. A child sends `IPC_WAITING_LOCK` before trying to enter a node, sends `IPC_INSIDE_NODE` after it gets the lock, stays inside the node for 1 second, releases the lock, and then sends its normal movement update. Entering and staying inside a node is the critical section.

For Milestone 5, we chose to use pipes:

- Pipes fit this milestone because the communication is one-way: the child processes only need to send progress updates to the parent. Each child sends small fixed-size messages containing its PID, current node, next node, and status. This keeps the implementation simple and avoids the extra synchronization that shared memory would require.

For Milestone 6, we chose to use System V semaphores:

- Normal mutexes are not enough here because the travelers are separate processes, not threads in the same process. System V semaphores are process-safe, so all child processes can use the same node locks. Each node semaphore starts at `1`, so only one traveler can be inside that node at a time.

## 4. How to Build & Run

### Milestone 1

```bash
make milestone1
./dijkstra <file-name>
```

### Milestone 2-5

```bash
make milestone5
./sim <file-name>
```

### Milestone 6

```bash
make milestone6
./sim tests/inputs/ms6_three_waiting.txt
```

### Milestone7

```bash
make milestone7
./sim -schd fcfs <file-name>

make milestone7
./sim -schd sjf <file-name>
```
