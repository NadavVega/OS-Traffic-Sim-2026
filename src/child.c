#include "child.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void run_child_process(void) {
  printf("[%d] started\n", getpid());
  fflush(stdout);

  for (;;) {
    pause();
  }
}
