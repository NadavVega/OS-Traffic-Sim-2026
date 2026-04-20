# OS Project: Directed Graph Traffic Simulation

## 1. Project Overview
This project is a semester-long simulation of a movement system within a directed, weighted graph. It is designed to demonstrate core Operating Systems concepts, including:
* **Concurrency**: Multiple "passengers" implemented as independent processes moving simultaneously.
* **IPC (Inter-Process Communication)**: Managing data flow and communication between active entities.
* **Synchronization & Scheduling**: Utilizing mutexes, semaphores, and specific scheduling algorithms to manage shared resources and timing.
* **Algorithms**: Efficient pathfinding using Dijkstra’s algorithm to find the shortest path between nodes.
* **Visualization**: A graphical interface built with the raylib library to display nodes, edges, and movement.

## 2. The Team: "The Schedulers"
In accordance with course guidelines, every team member is responsible for understanding the entire codebase and must be active in the repository. To ensure organized progress, we have divided primary development focus as follows:

| Name | Primary Module Focus | Core Responsibilities |
| :--- | :--- | :--- |
| **Nadav** | **Team Lead & Integration** | Project management, repository maintenance, and main execution flow. |
| **Binyamin** | **Parser & Input Logic** | File I/O operations and validation of the graph input format. |
| **Nave** | **Algorithmic Engine** | Implementation of the Dijkstra algorithm and path calculation. |
| **Bar** | **Data Architecture** | Memory management and graph data structures (Adjacency List/Matrix). |

## 3. Development Standards
* **Language**: C.
* **Platform**: Linux.
* **Version Control**: GitHub (Centralized repository with mandatory commits from all members).
* **Build System**: CMake.
* **Memory Safety**: Strict adherence to manual memory tracking to ensure no memory leaks; input validation for negative weights.
* **Coding Principles**: Adherence to modular programming and SOLID principles where applicable in C.

## 4. Project Structure & Milestones
The project is divided into seven key milestones to be completed throughout the semester:
1. **Milestone 1**: Graph representation and Dijkstra algorithm implementation.
2. **Milestone 2**: Graphical interface (GUI) to display the static graph with up to 15 nodes.
3. **Future Steps**: Movement animation, multi-processing, IPC, synchronization, and scheduling.

## 5. How to Build & Run
1. **Clone the repository**: `git clone [Your-Repo-URL]`
2. **Create build directory**: `mkdir build && cd build`
3. **Configure**: `cmake ..`
4. **Compile**: `make`
5. **Run**: `./OS_Project <input_file>`

---

* [Click here to watch the workflow](https://github.com/NadavVega/OS-Traffic-Sim-2026/wiki/Team-Git-&-GitHub-Workflow-Guide)
* [Click here to watch the CMake instructions](https://github.com/NadavVega/OS-Traffic-Sim-2026/wiki/CMake-&-Build-System-Guide)
