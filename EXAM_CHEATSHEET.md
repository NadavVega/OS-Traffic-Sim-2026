//============================================================
//------------------------ EXAM KEY WORDS --------------------
//============================================================

If the question says...

"change IPC message / add status"
→ ipc.h, ipc.c, child.c, main.c

"parent prints wrong log"
→ main.c → handle_ipc_message

"child behavior / sleep / route / request node"
→ child.c → run_child_process

"fork / process creation"
→ main.c → start_ms5_children

"waitpid / zombie"
→ main.c → reap_exited_children or terminate_and_wait_for_ms5_children

"signal"
→ main.c termination functions + child.c if child must handle signal

"semaphore / critical section / node lock"
→ node_locks.c + child.c Milestone 6 block

"FCFS / SJF / waiting queue"
→ scheduler.c + scheduler.h

"scheduler grant"
→ main.c → grant_next_waiting_traveler

"GUI state / waiting color / active scheduler text"
→ gui.c + main.c drawing section

"input format / validation"
→ parser.c

"shortest path / route cost"
→ dijkstra.c

"graph edge / node count / matrix"
→ graph.c / graph.h

//============================================================

File: main.c
Controls:

- Program entry point.
- CLI parsing.
- Input loading.
- GUI loop.
- Child process creation.
- IPC receiving.
- GUI entity updates.
- Milestone 6 semaphore creation/destruction.
- Milestone 7 scheduler creation/destruction.
- Milestone 7 private grant pipes.
- Child cleanup and waitpid.

Main functions:

- find_traveler_by_pid(...)
- grant_next_waiting_traveler(...)
- handle_ipc_message(...)
- all_ms5_children_finished(...)
- start_ms5_children(...)
- reap_exited_children(...)
- terminate_and_wait_for_ms5_children(...)
- all_ms4_children_finished(...)
- terminate_and_wait_for_child(...)
- main(...)

//============================================================

//============================================================

File: ipc.h / ipc.c
Controls:

- Pipe-based IPC message format.
- IPC status enum.
- Creating pipes.
- Setting read side non-blocking.
- Sending fixed-size messages.
- Reading messages from the pipe.

Main structs/enums:

- IpcStatus
- IpcMessage
- IpcReadResult

Main functions:

- ipc_create_pipe()
- ipc_set_nonblocking()
- ipc_send_message()
- ipc_read_message()

Exam keywords:
pipe, IPC, parent receives message, child sends message, non-blocking read,
new status, IPC_REQUEST_NODE, IPC_GRANTED_NODE, IPC_LEFT_NODE.

//============================================================

//============================================================

File: scheduler.h / scheduler.c
Controls:

- Milestone 7 scheduling logic.
- One waiting queue per graph node.
- FCFS and SJF selection.
- Node busy/free state.

Main structs:

- SchedulerState
- SchedulerItem
- SchedulerNodeItem (internal in scheduler.c)

Main functions:

- scheduler_create(node_count, algorithm)
- scheduler_destroy(scheduler)
- scheduler_add_waiting(...)
- scheduler_choose_next(...)
- scheduler_mark_node_busy(...)
- scheduler_mark_node_free(...)
- scheduler_is_node_busy(...)
- scheduler_parse_algorithm(...)
- scheduler_algorithm_name(...)

Exam keywords:
fcfs, sjf, waiting queue, node queue, shortest job first, next edge weight,
busy/free node, grant next traveler.

//============================================================

//============================================================

File: child.h / child.c
Controls:

- Code that runs inside each child process.
- Milestone 4: child prints started and pauses forever.
- Milestone 5: child calculates route and sends IPC movement updates.
- Milestone 6: child locks node with semaphore.
- Milestone 7: child requests permission from parent scheduler and waits for grant.

Main function:

- run_child_process(...)

Exam keywords:
child process, Dijkstra inside child, send IPC, wait for grant,
sleep inside node, edge travel time, no path, finished.

//============================================================

//============================================================

File: node_locks.h / node_locks.c
Controls:

- System V semaphore set.
- One semaphore per graph node.
- Locking/unlocking nodes in Milestone 6.

Main functions:

- node_locks_create(node_count)
- node_locks_lock(semaphore_id, node_index)
- node_locks_unlock(semaphore_id, node_index)
- node_locks_destroy(semaphore_id)

Exam keywords:
semaphore, critical section, mutual exclusion, one traveler inside node,
deadlock, starvation, node lock.

//============================================================

//============================================================

File: parser.h / parser.c
Controls:

- Reads input file.
- Creates Graph.
- Reads edges.
- Reads travelers.
- Validates node indexes and negative weights.
- Assigns traveler colors.

Main struct:

- Traveler

Main function:

- parse_graph_from_file(filename, &travelers, &num_travelers)

Helper functions:

- read_data_line()
- parse_two_ints()
- parse_three_ints()
- parse_one_int()

Exam keywords:
input format, invalid node, negative weight, number of travelers,
comments/blank lines, traveler source/destination.

//============================================================

//============================================================

File: graph.h / graph.c
Controls:

- Graph struct.
- Adjacency matrix.
- Node count and edge count.
- Basic graph memory.
- Older AnimationState support.

Main functions:

- create_graph(nodes, edges)
- add_edge(g, src, dest, weight)
- free_graph(g)
- calculate_node_positions(...)
- init_animation(...)
- free_animation(...)

Exam keywords:
adjacency matrix, MAX_NODES, edge weight, no edge = -1, graph memory.

//============================================================

//============================================================

File: dijkstra.h / dijkstra.c
Controls:

- Shortest path calculation.
- Path reconstruction.
- Total path weight.

Main struct:

- dijkstraResult

Main functions:

- find_min_distance(...)
- find_shortest_path(num_nodes, graph, start, end)

Exam keywords:
Dijkstra, shortest path, no path, source == destination, total weight.

//============================================================

//============================================================

File: gui.h / gui.c
Controls:

- Raylib drawing.
- Node positions.
- Static graph drawing.
- Play/Stop button.
- Completion message.
- Traveler visual state and movement.

Main structs:

- VisualNode
- Entity
- EntityVisualState

Main functions:

- InitGraphVisuals(...)
- DrawStaticGraph(...)
- DrawButton(...)
- DrawCompletionMessage(...)
- UpdateEntities(...)
- UpdateIpcEntities(...)
- DrawEntities(...)

Exam keywords:
GUI, Raylib, draw graph, draw travelers, waiting color, inside node,
movement duration, scheduler name display.

//============================================================
