#include "node_locks.h"

#include <errno.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

static int node_locks_validate_index(int node_index) {
  if (node_index < 0) {
    fprintf(stderr, "Error: Invalid node lock index %d\n", node_index);
    return -1;
  }
  return 0;
}

int node_locks_create(int node_count) {
  if (node_count <= 0) {
    fprintf(stderr, "Error: Node lock count must be positive.\n");
    return -1;
  }

  int semaphore_id = semget(IPC_PRIVATE, node_count, IPC_CREAT | 0600);
  if (semaphore_id == -1) {
    perror("Error: Failed to create node lock semaphore set");
    return -1;
  }

  union semun arg;
  arg.val = 1;
  for (int i = 0; i < node_count; i++) {
    if (semctl(semaphore_id, i, SETVAL, arg) == -1) {
      perror("Error: Failed to initialize node lock semaphore");
      if (semctl(semaphore_id, 0, IPC_RMID) == -1) {
        perror("Error: Failed to remove partially initialized node locks");
      }
      return -1;
    }
  }

  return semaphore_id;
}

int node_locks_lock(int semaphore_id, int node_index) {
  if (node_locks_validate_index(node_index) == -1) {
    return -1;
  }

  struct sembuf operation = {
      .sem_num = (unsigned short)node_index,
      .sem_op = -1,
      .sem_flg = 0,
  };

  while (semop(semaphore_id, &operation, 1) == -1) {
    if (errno != EINTR) {
      perror("Error: Failed to lock node semaphore");
      return -1;
    }
  }

  return 0;
}

int node_locks_unlock(int semaphore_id, int node_index) {
  if (node_locks_validate_index(node_index) == -1) {
    return -1;
  }

  struct sembuf operation = {
      .sem_num = (unsigned short)node_index,
      .sem_op = 1,
      .sem_flg = 0,
  };

  while (semop(semaphore_id, &operation, 1) == -1) {
    if (errno != EINTR) {
      perror("Error: Failed to unlock node semaphore");
      return -1;
    }
  }

  return 0;
}

int node_locks_destroy(int semaphore_id) {
  if (semctl(semaphore_id, 0, IPC_RMID) == -1) {
    perror("Error: Failed to destroy node lock semaphore set");
    return -1;
  }

  return 0;
}
