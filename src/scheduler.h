#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>
#include <sys/types.h>

typedef enum {
  SCHEDULER_FCFS = 0,
  SCHEDULER_SJF = 1
} SchedulerAlgorithm;

typedef struct SchedulerState SchedulerState;

typedef struct {
  int traveler_index;
  pid_t pid;
  int node;
  int next_node;
  int job_length;
} SchedulerItem;

SchedulerState *scheduler_create(int node_count, SchedulerAlgorithm algorithm);
void scheduler_destroy(SchedulerState *scheduler);

int scheduler_add_waiting(SchedulerState *scheduler, int node,
                          int traveler_index, pid_t pid, int next_node,
                          int job_length);
int scheduler_choose_next(SchedulerState *scheduler, int node,
                          SchedulerItem *selected);

int scheduler_mark_node_busy(SchedulerState *scheduler, int node);
int scheduler_mark_node_free(SchedulerState *scheduler, int node);
bool scheduler_is_node_busy(const SchedulerState *scheduler, int node);

int scheduler_parse_algorithm(const char *name, SchedulerAlgorithm *algorithm);
const char *scheduler_algorithm_name(SchedulerAlgorithm algorithm);

#endif // SCHEDULER_H
