#include "parser.h"
#include "gui.h" // For travelerColors array
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//===========================================================
// parse_graph_from_file:
// Receives filename, Traveler** and int*.
// Allocates graph and travelers array.
// Returns Graph* on success, NULL on error.
//===========================================================

#define INPUT_LINE_SIZE 256

static bool read_data_line(FILE *fp, char *line, size_t size,
                           int *line_number) {
  while (fgets(line, size, fp) != NULL) {
    (*line_number)++;

    size_t length = 0;
    while (line[length] != '\0' && line[length] != '\n') {
      length++;
    }
    if (line[length] != '\n' && !feof(fp)) {
      int ch;
      while ((ch = fgetc(fp)) != '\n' && ch != EOF) {
      }
      fprintf(stderr, "Error: Input line %d is too long.\n", *line_number);
      return false;
    }

    char *content = line;
    while (isspace((unsigned char)*content)) {
      content++;
    }
    if (*content == '\0' || *content == '#') {
      continue;
    }

    return true;
  }

  return false;
}

static bool parse_two_ints(const char *line, int *first, int *second) {
  char extra;
  return sscanf(line, " %d %d %c", first, second, &extra) == 2;
}

static bool parse_three_ints(const char *line, int *first, int *second,
                             int *third) {
  char extra;
  return sscanf(line, " %d %d %d %c", first, second, third, &extra) == 3;
}

static bool parse_one_int(const char *line, int *value) {
  char extra;
  return sscanf(line, " %d %c", value, &extra) == 1;
}

Graph *parse_graph_from_file(const char *filename, Traveler **travelers,
                             int *num_travelers) {
  if (filename == NULL || travelers == NULL || num_travelers == NULL) {
    fprintf(stderr, "Error: Invalid parser arguments.\n");
    return NULL;
  }

  *travelers = NULL;
  *num_travelers = 0;

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    perror("Error opening file");
    return NULL;
  }

  char line[INPUT_LINE_SIZE];
  int line_number = 0;
  int nodes, edges;

  if (!read_data_line(fp, line, sizeof(line), &line_number) ||
      !parse_two_ints(line, &nodes, &edges)) {
    fprintf(stderr,
            "Error: Expected node and edge counts on the first data line.\n");
    fclose(fp);
    return NULL;
  }
  if (nodes <= 0 || nodes > MAX_NODES || edges < 0) {
    fprintf(stderr,
            "Error: Invalid graph size at line %d (nodes: %d, edges: %d).\n",
            line_number, nodes, edges);
    fclose(fp);
    return NULL;
  }

  Graph *g = create_graph(nodes, edges);
  if (g == NULL) {
    fprintf(stderr, "Error: Failed to create graph.\n");
    fclose(fp);
    return NULL;
  }

  for (int i = 0; i < edges; i++) {
    int u, v, w;
    if (!read_data_line(fp, line, sizeof(line), &line_number) ||
        !parse_three_ints(line, &u, &v, &w)) {
      fprintf(stderr,
              "Error: Expected edge %d as: source destination weight.\n",
              i + 1);
      free_graph(g);
      fclose(fp);
      return NULL;
    }
    if (u < 0 || u >= nodes || v < 0 || v >= nodes) {
      fprintf(stderr,
              "Error: Edge %d at line %d uses node index outside 0-%d.\n",
              i + 1, line_number, nodes - 1);
      free_graph(g);
      fclose(fp);
      return NULL;
    }
    if (w < 0) {
      fprintf(stderr, "Error: Edge %d at line %d has a negative weight.\n",
              i + 1, line_number);
      free_graph(g);
      fclose(fp);
      return NULL;
    }
    add_edge(g, u, v, w);
  }

  if (!read_data_line(fp, line, sizeof(line), &line_number) ||
      !parse_one_int(line, num_travelers)) {
    fprintf(stderr,
            "Error: Expected the number of travelers after the edges.\n");
    free_graph(g);
    fclose(fp);
    return NULL;
  }
  if (*num_travelers <= 0) {
    fprintf(stderr, "Error: Traveler count at line %d must be positive.\n",
            line_number);
    free_graph(g);
    fclose(fp);
    return NULL;
  }

  *travelers = malloc((size_t)*num_travelers * sizeof(Traveler));
  if (*travelers == NULL) {
    fprintf(stderr, "Error: Failed to allocate traveler data.\n");
    free_graph(g);
    fclose(fp);
    return NULL;
  }

  for (int i = 0; i < *num_travelers; i++) {
    int src, dest;
    if (!read_data_line(fp, line, sizeof(line), &line_number) ||
        !parse_two_ints(line, &src, &dest)) {
      fprintf(stderr, "Error: Expected traveler %d as: source destination.\n",
              i + 1);
      free(*travelers);
      *travelers = NULL;
      *num_travelers = 0;
      free_graph(g);
      fclose(fp);
      return NULL;
    }
    if (src < 0 || src >= nodes || dest < 0 || dest >= nodes) {
      fprintf(stderr,
              "Error: Traveler %d at line %d uses node index outside 0-%d.\n",
              i + 1, line_number, nodes - 1);
      free(*travelers);
      *travelers = NULL;
      *num_travelers = 0;
      free_graph(g);
      fclose(fp);
      return NULL;
    }

    (*travelers)[i].src = src;
    (*travelers)[i].dest = dest;
    (*travelers)[i].pid = 0;
    (*travelers)[i].path = NULL;
    (*travelers)[i].path_length = 0;
    (*travelers)[i].current_path_index = 0;
    (*travelers)[i].color = travelerColors[i % 15];
  }

  fclose(fp);
  return g;
}
