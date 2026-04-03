#include <stdio.h>
#include "reroute.h"
#include "bfs.h"
#include "astar.h"
#include "dijkstra.h"

void rerouteAfterBlockage(Graph *g, int roadSrc, int roadDest,
                          int evacuee, int shelter)
{
    LOG("\n╔══════════════════════════════════════╗\n");
    LOG("║      DYNAMIC REROUTING TRIGGERED     ║\n");
    LOG("╚══════════════════════════════════════╝\n");

    // Step 1: Block the road
    blockEdge(g, roadSrc, roadDest);

    // Step 2: Check if shelter is still reachable
    LOG("\n[Reroute] Checking if '%s' is still reachable from '%s'...\n",
        g->nodes[shelter].name, g->nodes[evacuee].name);

    if (!bfsPathExists(g, evacuee, shelter))
    {
        LOG("[CRITICAL] ❌ '%s' is now UNREACHABLE from '%s'!\n",
            g->nodes[shelter].name, g->nodes[evacuee].name);
        LOG("[Reroute] Scanning all shelters for alternatives...\n\n");

        // Try every other shelter
        int found = 0;
        for (int i = 0; i < g->count; i++)
        {
            if (g->nodes[i].type == SHELTER && i != shelter)
            {
                if (bfsPathExists(g, evacuee, i))
                {
                    LOG("[Reroute] ✓ Alternative shelter found: '%s'\n",
                        g->nodes[i].name);
                    int path[MAX_NODES], pathLen;
                    astar(g, evacuee, i, path, &pathLen);
                    found = 1;
                    break;
                }
            }
        }
        if (!found)
            LOG("[CRITICAL] ❌ NO shelters reachable! Area is completely cut off.\n");
        return;
    }

    // Step 3: Re-run A* for new optimal path
    LOG("[Reroute] Path exists — finding new optimal route via A*...\n");
    int newPath[MAX_NODES], newLen;
    int newDist = astar(g, evacuee, shelter, newPath, &newLen);

    LOG("\n[Reroute] ✓ Rerouting complete. New distance: %d units\n", newDist);
}