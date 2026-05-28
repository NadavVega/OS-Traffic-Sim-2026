# OS Project: Directed Graph Traffic Simulation - "The Schedulers"

## 1. Project Overview

A simulation of movement within a directed graph, exploring OS concepts such as concurrency, IPC, and synchronization.

## 2. The Team

| Name         | Responsibilities                                                                                                                                                                                                 |
| :----------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Nadav**    | Managed the OS-level creation of child processes (`fork`), handled their delay/hold states (`pause`), and ensured proper closure using signals to prevent zombie processes (`main.c`).                           |
| **Nave**     | Upgraded the GUI to a multi-traveler display, rendering each entity simultaneously with unique colors (`gui.h`, `gui.c`).                                                                                        |
| **Bar**      | Implemented the file parsing logic to dynamically read and allocate an array of multiple travelers (`parser.h`, `parser.c`).                                                                                     |
| **Binyamin** | Connected all modules, managed the main Raylib GUI loop, updated the `Makefile`, and ensured all project assignment requirements were met. **(Nadav pushed his code because he is it reserve duty at the time)** |

## 3. Implementation Description

- **Milestone 1**: Implemented Dijkstra's algorithm using an adjacency matrix. Handles directed edges and weight calculations.
- **Milestone 2**: Created a circular layout for nodes. Visualized edges with directed arrows and legible weights using Raylib.
- **Milestone 3**: Added a golden entity that moves along the calculated path. Implemented a 1-second pause at nodes and 300ms jumps per weight unit.
- **Milestone 4**: Transitioned to a multi-process architecture where multiple travelers move concurrently. The parent process computes paths and drives the GUI, while child processes represent the travelers.

## 4. How to Build & Run

### Milestone 1

```bash
make milestone1
./dijkstra <file-name>
```

### Milestone2-6

```bash
make milestone2-6
./sim <file-name>
```

### Milestone7

```bash
make milestone7
./sim -schd fcfs <file-name>

make milestone7
./sim -schd sjf <file-name>
```
