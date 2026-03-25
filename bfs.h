#ifndef BFS_H
#define BFS_H
#include "graph.h"

// BFS from a disaster origin — marks all reachable nodes within 'danger_radius' hops
// Returns how many zones are affected
int bfsFloodZones(Graph *g, int origin, int danger_radius, int *affected);

// BFS to check if a path EXISTS between src and dest (ignoring weights)
int bfsPathExists(Graph *g, int src, int dest);

#endif