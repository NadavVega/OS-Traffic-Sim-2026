#include "ipc.h"
#include <errno.h>
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

int ipc_read_message(int read_fd, IpcMessage *message) {
  size_t total_read = 0;
  unsigned char *buffer = (unsigned char *)message;

  while (total_read < sizeof(*message)) {
    ssize_t bytes_read =
        read(read_fd, buffer + total_read, sizeof(*message) - total_read);
    if (bytes_read == 0) {
      if (total_read == 0) {
        return 0;
      }
      fprintf(stderr, "Error: Incomplete IPC message read.\n");
      return -1;
    }
    if (bytes_read == -1) {
      if (errno == EINTR) {
        continue;
      }
      perror("Error: Failed to read IPC message");
      return -1;
    }
    total_read += (size_t)bytes_read;
  }

  return 1;
}
