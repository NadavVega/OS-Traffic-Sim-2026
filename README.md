# OS Project: Directed Graph Traffic Simulation - "The Schedulers"

## 1. Project Overview

A simulation of movement within a directed graph, exploring OS concepts such as concurrency, IPC, and synchronization.

## 2. The Team

| Name         | Role                   | Responsibilities                                    |
| :----------- | :--------------------- | :-------------------------------------------------- |
| **Nadav**    | **Algorithmic Engine** | Dijkstra implementation & shortest path logic.      |
| **Nave**     | **GUI Developer**      | Raylib integration, graph rendering, and animation. |
| **Bar**      | **System Integration** | Flow control, File I/O, and connecting modules.     |
| **Binyamin** | **Reserve Duty**       | Will join for IPC/Synchronization modules.          |

## 3. Implementation Description

- **Milestone 1**: Implemented Dijkstra's algorithm using an adjacency matrix. Handles directed edges and weight calculations.
- **Milestone 2**: Created a circular layout for nodes. Visualized edges with directed arrows and legible weights using Raylib.
- **Milestone 3**: Added a golden entity that moves along the calculated path. Implemented a 1-second pause at nodes and 300ms jumps per weight unit.

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
