#include <stdio.h>
#include "reroute.h"
#include "bfs.h"
#include "astar.h"
#include "dijkstra.h"

/*
 * DYNAMIC REROUTING
 * -----------------
 * Real disasters = roads collapse, flood, or get blocked in real-time.
 * Static pre-planned routes become invalid.
 *
 * Strategy:
 *   Step 1: Block the road (mark edge as blocked)
 *   Step 2: BFS connectivity check — O(V+E), fast sanity check
 *   Step 3: If reachable → A* gives new optimal path quickly
 *   Step 4: If completely cut off → alert, try all shelters
 *
 * Why A* for rerouting instead of Dijkstra?
 *   - In an emergency, SPEED matters
 *   - A* explores fewer nodes due to heuristic guidance
 *   - Both return optimal paths; A* just gets there faster
 */

void rerouteAfterBlockage(Graph *g, int roadSrc, int roadDest,
                          int evacuee, int shelter)
{
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║      DYNAMIC REROUTING TRIGGERED     ║\n");
    printf("╚══════════════════════════════════════╝\n");

    // Step 1: Block the road
    blockEdge(g, roadSrc, roadDest);

    // Step 2: Check if shelter is still reachable
    printf("\n[Reroute] Checking if '%s' is still reachable from '%s'...\n",
           g->nodes[shelter].name, g->nodes[evacuee].name);

    if (!bfsPathExists(g, evacuee, shelter))
    {
        printf("[CRITICAL] ❌ '%s' is now UNREACHABLE from '%s'!\n",
               g->nodes[shelter].name, g->nodes[evacuee].name);
        printf("[Reroute] Scanning all shelters for alternatives...\n\n");

        // Try every other shelter
        int found = 0;
        for (int i = 0; i < g->count; i++)
        {
            if (g->nodes[i].type == SHELTER && i != shelter)
            {
                if (bfsPathExists(g, evacuee, i))
                {
                    printf("[Reroute] ✓ Alternative shelter found: '%s'\n",
                           g->nodes[i].name);
                    int path[MAX_NODES], pathLen;
                    astar(g, evacuee, i, path, &pathLen);
                    found = 1;
                    break;
                }
            }
        }
        if (!found)
            printf("[CRITICAL] ❌ NO shelters reachable! Area is completely cut off.\n");
        return;
    }

    // Step 3: Re-run A* for new optimal path
    printf("[Reroute] Path exists — finding new optimal route via A*...\n");
    int newPath[MAX_NODES], newLen;
    int newDist = astar(g, evacuee, shelter, newPath, &newLen);

    printf("\n[Reroute] ✓ Rerouting complete. New distance: %d units\n", newDist);
}