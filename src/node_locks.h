#ifndef NODE_LOCKS_H
#define NODE_LOCKS_H

int node_locks_create(int node_count);
int node_locks_lock(int semaphore_id, int node_index);
int node_locks_unlock(int semaphore_id, int node_index);
int node_locks_destroy(int semaphore_id);

#endif // NODE_LOCKS_H
