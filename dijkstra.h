#ifndef DIJKSTRA_H
#define DIJKSTRA_H
#include "graph.h"

// Returns shortest distance from src to dest, prints full path
// Returns INF if no path exists
int dijkstra(Graph *g, int src, int dest, int *path, int *pathLen);

#endif