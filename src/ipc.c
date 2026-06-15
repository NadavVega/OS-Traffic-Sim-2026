#include "ipc.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

// All children share one pipe; messages this small are written atomically.
_Static_assert(sizeof(IpcMessage) <= PIPE_BUF,
               "IpcMessage must fit in one atomic pipe write");

int ipc_create_pipe(int pipe_fd[2]) {
  if (pipe(pipe_fd) == -1) {
    perror("Error: Failed to create IPC pipe");
    return -1;
  }
  return 0;
}

int ipc_set_nonblocking(int read_fd) {
  int flags = fcntl(read_fd, F_GETFL);
  if (flags == -1 || fcntl(read_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("Error: Failed to configure IPC pipe");
    return -1;
  }
  return 0;
}

int ipc_send_message(int write_fd, const IpcMessage *message) {
  ssize_t bytes_written;
  do {
    bytes_written = write(write_fd, message, sizeof(*message));
  } while (bytes_written == -1 && errno == EINTR);

  if (bytes_written != (ssize_t)sizeof(*message)) {
    if (bytes_written == -1) {
      perror("Error: Failed to send IPC message");
    } else {
      fprintf(stderr, "Error: Incomplete IPC message write.\n");
    }
    return -1;
  }
  return 0;
}

IpcReadResult ipc_read_message(int read_fd, IpcMessage *message) {
  ssize_t bytes_read;
  do {
    bytes_read = read(read_fd, message, sizeof(*message));
  } while (bytes_read == -1 && errno == EINTR);

  if (bytes_read == (ssize_t)sizeof(*message)) {
    return IPC_READ_MESSAGE;
  }
  if (bytes_read == 0) {
    return IPC_READ_EOF;
  }
  if (bytes_read == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return IPC_READ_EMPTY;
    }
    perror("Error: Failed to read IPC message");
    return IPC_READ_ERROR;
  }

  fprintf(stderr, "Error: Incomplete IPC message read.\n");
  return IPC_READ_ERROR;
}
