#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

typedef enum {
  IPC_EN_ROUTE = 0,
  IPC_ARRIVED_DEST = 1,
  IPC_FINISHED = 2
} IpcStatus;

typedef struct {
  pid_t pid;
  int current_node;
  int next_node;
  IpcStatus status;
} IpcMessage;

int ipc_create_pipe(int pipe_fd[2]);
int ipc_send_message(int write_fd, const IpcMessage *message);
int ipc_read_message(int read_fd, IpcMessage *message);

#endif // IPC_H
