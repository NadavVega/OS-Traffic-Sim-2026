#ifndef PARSER_H
#define PARSER_H

#include "graph.h"

Graph* parse_graph_from_file(const char* filename, int* src, int* dest);

#endif