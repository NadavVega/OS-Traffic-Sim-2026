#ifndef IPC_H
#define IPC_H
#include <sys/types.h>
#include <unistd.h>

typedef struct {
  pid_t child_pid;
  int current_node;
  int next_node;
  int is_finished;
} ipc_message_t;

int init_ipc(int pipe_fd[2]);
int send_message(int write_fd, ipc_message_t *message);
int receive_message(int read_fd, ipc_message_t *message);

#endif // IPC_H