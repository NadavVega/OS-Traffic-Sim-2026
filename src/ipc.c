#include "ipc.h"
#include <errno.h>
// #include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int init_ipc(int pipe_fd[2]) {
  if (pipe(pipe_fd) == -1) {
    perror("pipe in init_ipc");
    return -1;
  }

  int flags = fcntl(pipe_fd[0], F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl F_GETFL in init_ipc");
    return -1;
  }
  if (fcntl(pipe_fd[0], F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL in init_ipc");
    return -1;
  }
  return 0;
}

int send_message(int write_fd, ipc_message_t *message) {
  if (write(write_fd, message, sizeof(ipc_message_t)) == -1) {
    perror("write in send_message");
    return -1;
  }
  return 0;
}

int receive_message(int read_fd, ipc_message_t *message) {
  ssize_t bytesRead = read(read_fd, message, sizeof(ipc_message_t));
  if (bytesRead == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // No data available, non-blocking read
      return 0;
    }
    perror("read in receive_message");
    return -1;
  } else if (bytesRead == 0) {
    // EOF
    return 0;
  }
  return 1; // Successfully read a message
}