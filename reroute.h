#ifndef REROUTE_H
#define REROUTE_H
#include "graph.h"

/*
 * Dynamic Rerouting
 * -----------------
 * When a road is blocked mid-evacuation, we need to instantly
 * find an alternative route. This module:
 *   1. Blocks the specified edge
 *   2. Checks if destination is still reachable (BFS)
 *   3. Re-runs A* (faster) to find the new best path
 *   4. Compares old vs new route cost
 */
void rerouteAfterBlockage(Graph *g, int roadSrc, int roadDest,
                          int evacuee, int shelter);

#endif