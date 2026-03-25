#ifndef ASTAR_H
#define ASTAR_H
#include "graph.h"

// A* Search: uses Euclidean distance heuristic (x,y coords on node)
// Returns shortest distance, fills path[]
int astar(Graph *g, int src, int dest, int *path, int *pathLen);

#endif