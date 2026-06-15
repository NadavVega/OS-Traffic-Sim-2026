# OS Traffic Simulation

This project simulates multiple travelers moving through a directed, weighted
graph. It demonstrates process creation, Dijkstra shortest-path calculation,
signals, inter-process communication (IPC), child cleanup, and a Raylib GUI.

## Build

Build Milestone 1:

```bash
make milestone1
```

Build Milestone 2:

```bash
make milestone2
```

Build Milestone 3:

```bash
make milestone3
```

Build Milestone 4:

```bash
make milestone4
```

Build Milestone 5:

```bash
make milestone5
```

Remove generated executables and object files:

```bash
make clean
```

Milestone 1 produces `dijkstra`. Milestones 2 through 5 produce `sim`.

## Run

```bash
./sim <file_name>
```

Milestone 1 uses:

```bash
./dijkstra <file_name>
```

For example:

```bash
./sim test_m4.txt
```

Raylib requires access to a graphical display.

## Input Format

Blank lines are allowed. Lines beginning with `#`, including lines with
whitespace before `#`, are ignored.

The data lines contain:

1. Number of nodes and number of directed edges.
2. One `source destination weight` line per edge.
3. Number of travelers.
4. One `source destination` line per traveler.

Example:

```txt
# graph definition
5 7
0 1 4
0 2 2
1 3 5
2 1 1
2 3 8
3 4 2
1 4 6

# travelers
2
0 4
2 3
```

Node indexes must be within the graph's valid range. Invalid indexes, malformed
lines, negative weights, and unsupported graph sizes are rejected safely.

## Milestones 1-3

- Milestone 1 implements Dijkstra shortest-path calculation for a directed,
  weighted graph stored as an adjacency matrix.
- Milestone 2 draws the graph with Raylib, including directed edges, nodes, and
  edge weights.
- Milestone 3 animates a traveler along its route with edge travel timing and
  node waits. Its visible Play/Stop button starts, pauses, and resumes the
  animation. The initial state is paused.

## Milestone 4

Milestone 4 uses parent-controlled routes and GUI movement:

- The parent parses the graph and traveler definitions.
- The parent calculates each Dijkstra route.
- The parent forks one child process per traveler.
- Each child prints `[PID] started` and waits for the parent.
- The parent controls traveler movement in the GUI.
- The initial GUI state is paused. Play starts movement, Stop pauses it, and
  Play resumes it.
- When a traveler completes its route, the parent sends that child's PID
  `SIGTERM`.
- The parent waits for and reaps every child before exiting.
- After all routes complete, the final traveler positions and a completion
  message remain visible until the user closes the window normally.
- If the GUI closes early or cannot initialize, remaining children are
  terminated and reaped.

## Milestone 5

Milestone 5 uses autonomous children and IPC-driven GUI updates:

- The parent parses the graph and traveler definitions.
- The parent forks one child process per traveler when Play is pressed for the
  first time.
- Children do not print `[PID] started`.
- Each child calculates its own Dijkstra route using the inherited graph.
- Each child reports node progress to the parent through a pipe.
- The parent reads IPC messages, prints the required terminal logs, and updates
  the matching GUI entity.
- MS5 GUI positions change only when IPC node reports arrive; the parent does
  not independently calculate or advance routes.
- Stop freezes visible GUI positions. The parent continues draining and storing
  IPC reports so children cannot block on a full pipe. Play applies the latest
  reported positions and resumes visible updates.
- The parent waits for and reaps completed children.
- After every child reports completion and is reaped, the final destination
  state and completion message remain visible until the user closes the window
  normally.
- On GUI close or initialization failure, remaining children are terminated and
  reaped.

## IPC Design

Milestone 5 uses one shared pipe for one-way child-to-parent progress reports.
Pipes fit this workflow because the parent only needs to receive updates from
its children. They also avoid the synchronization and overwrite concerns of
shared memory.

Each child writes fixed-size `IpcMessage` values:

```c
typedef enum {
    IPC_EN_ROUTE,
    IPC_ARRIVED_DEST,
    IPC_FINISHED
} IpcStatus;

typedef struct {
    pid_t pid;
    int current_node;
    int next_node;
    IpcStatus status;
} IpcMessage;
```

- `pid` identifies the child and its corresponding traveler.
- `current_node` is the node the child reached.
- `next_node` is the planned next node, or `-1` at destination and finish.
- `status` tells the parent whether the child is en route, at its destination,
  or finished.

`IpcMessage` is checked to fit within `PIPE_BUF`. Each child sends a complete
message with one `write()` call, so concurrent writes remain atomic. The parent
uses a non-blocking read end and drains available complete messages during each
GUI frame.

## Milestone 5 Logs

Only the parent prints Milestone 5 progress logs:

```txt
[PID=X] arrived at node Y | next node: Z
[PID=X] arrived at node Y | DESTINATION
[PID=X] finished
```
