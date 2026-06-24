#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SchedulerNodeItem {
  SchedulerItem item;
  struct SchedulerNodeItem *next;
} SchedulerNodeItem;

struct SchedulerState {
  int node_count;
  SchedulerAlgorithm algorithm;
  SchedulerNodeItem **queues;
  bool *node_busy;
};

static bool scheduler_valid_node(const SchedulerState *scheduler, int node) {
  return scheduler != NULL && node >= 0 && node < scheduler->node_count;
}

SchedulerState *scheduler_create(int node_count, SchedulerAlgorithm algorithm) {
  if (node_count <= 0) {
    fprintf(stderr, "Error: Scheduler node count must be positive.\n");
    return NULL;
  }

  SchedulerState *scheduler = calloc(1, sizeof(*scheduler));
  if (scheduler == NULL) {
    return NULL;
  }

  scheduler->queues = calloc((size_t)node_count, sizeof(*scheduler->queues));
  scheduler->node_busy = calloc((size_t)node_count, sizeof(*scheduler->node_busy));
  if (scheduler->queues == NULL || scheduler->node_busy == NULL) {
    scheduler_destroy(scheduler);
    return NULL;
  }

  scheduler->node_count = node_count;
  scheduler->algorithm = algorithm;
  return scheduler;
}

void scheduler_destroy(SchedulerState *scheduler) {
  if (scheduler == NULL) {
    return;
  }

  if (scheduler->queues != NULL) {
    for (int i = 0; i < scheduler->node_count; i++) {
      SchedulerNodeItem *current = scheduler->queues[i];
      while (current != NULL) {
        SchedulerNodeItem *next = current->next;
        free(current);
        current = next;
      }
    }
  }

  free(scheduler->queues);
  free(scheduler->node_busy);
  free(scheduler);
}

int scheduler_add_waiting(SchedulerState *scheduler, int node,
                          int traveler_index, pid_t pid, int next_node,
                          int job_length) {
  if (!scheduler_valid_node(scheduler, node) || traveler_index < 0) {
    return -1;
  }

  SchedulerNodeItem *entry = malloc(sizeof(*entry));
  if (entry == NULL) {
    return -1;
  }

  entry->item.traveler_index = traveler_index;
  entry->item.pid = pid;
  entry->item.node = node;
  entry->item.next_node = next_node;
  entry->item.job_length = job_length;
  entry->next = NULL;

  SchedulerNodeItem **head = &scheduler->queues[node];
  if (*head == NULL) {
    *head = entry;
    return 0;
  }

  SchedulerNodeItem *tail = *head;
  while (tail->next != NULL) {
    tail = tail->next;
  }
  tail->next = entry;
  return 0;
}

int scheduler_choose_next(SchedulerState *scheduler, int node,
                          SchedulerItem *selected) {
  if (!scheduler_valid_node(scheduler, node) || selected == NULL) {
    return -1;
  }

  SchedulerNodeItem **chosen_link = &scheduler->queues[node];
  if (*chosen_link == NULL) {
    return 0;
  }

  if (scheduler->algorithm == SCHEDULER_SJF) {
    for (SchedulerNodeItem **link = &scheduler->queues[node]; *link != NULL;
         link = &(*link)->next) {
      if ((*link)->item.job_length < (*chosen_link)->item.job_length) {
        chosen_link = link;
      }
    }
  }

  SchedulerNodeItem *chosen = *chosen_link;
  *selected = chosen->item;
  *chosen_link = chosen->next;
  free(chosen);
  return 1;
}

int scheduler_mark_node_busy(SchedulerState *scheduler, int node) {
  if (!scheduler_valid_node(scheduler, node)) {
    return -1;
  }
  scheduler->node_busy[node] = true;
  return 0;
}

int scheduler_mark_node_free(SchedulerState *scheduler, int node) {
  if (!scheduler_valid_node(scheduler, node)) {
    return -1;
  }
  scheduler->node_busy[node] = false;
  return 0;
}

bool scheduler_is_node_busy(const SchedulerState *scheduler, int node) {
  if (!scheduler_valid_node(scheduler, node)) {
    return false;
  }
  return scheduler->node_busy[node];
}

int scheduler_parse_algorithm(const char *name, SchedulerAlgorithm *algorithm) {
  if (name == NULL || algorithm == NULL) {
    return -1;
  }

  if (strcmp(name, "fcfs") == 0 || strcmp(name, "FCFS") == 0) {
    *algorithm = SCHEDULER_FCFS;
    return 0;
  }
  if (strcmp(name, "sjf") == 0 || strcmp(name, "SJF") == 0) {
    *algorithm = SCHEDULER_SJF;
    return 0;
  }

  return -1;
}

const char *scheduler_algorithm_name(SchedulerAlgorithm algorithm) {
  switch (algorithm) {
  case SCHEDULER_FCFS:
    return "fcfs";
  case SCHEDULER_SJF:
    return "sjf";
  }
  return "unknown";
}
