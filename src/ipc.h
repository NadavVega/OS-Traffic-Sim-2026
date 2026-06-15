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

typedef enum {
  IPC_READ_ERROR = -1,
  IPC_READ_EMPTY = 0,
  IPC_READ_MESSAGE = 1,
  IPC_READ_EOF = 2
} IpcReadResult;

int ipc_create_pipe(int pipe_fd[2]);
int ipc_set_nonblocking(int read_fd);
int ipc_send_message(int write_fd, const IpcMessage *message);
IpcReadResult ipc_read_message(int read_fd, IpcMessage *message);

#endif // IPC_H
