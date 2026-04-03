#include <stdio.h>
#include <string.h>
#include "bfs.h"

int bfsFloodZones(Graph *g, int origin, int danger_radius, int *affected)
{
    int visited[MAX_NODES] = {0};
    int depth[MAX_NODES];
    int queue[MAX_NODES];
    int front = 0, rear = 0, count = 0;

    memset(depth, 0, sizeof(depth));

    visited[origin] = 1;
    queue[rear++] = origin;

    LOG("\n[BFS] Disaster origin: '%s'\n", g->nodes[origin].name);
    LOG("[BFS] Calculating danger zones (radius = %d hops)...\n\n", danger_radius);

    while (front < rear)
    {
        int curr = queue[front++];

        if (depth[curr] > danger_radius)
            continue;

        // Mark as affected (except the disaster origin itself)
        if (curr != origin)
        {
            affected[count++] = curr;
            LOG("  ⚠ DANGER: '%s' [%d hop(s) away]\n",
                g->nodes[curr].name, depth[curr]);
        }

        // Explore neighbors
        for (Edge *e = g->nodes[curr].head; e; e = e->next)
        {
            if (!visited[e->dest] && !e->blocked)
            {
                visited[e->dest] = 1;
                depth[e->dest] = depth[curr] + 1;
                queue[rear++] = e->dest;
            }
        }
    }

    LOG("\n[BFS] Total affected locations: %d\n", count);
    return count;
}

/*
 * BFS connectivity check — used before Dijkstra/A* to confirm
 * a route even EXISTS (handles disconnected graphs / all roads blocked)
 * Time complexity: O(V + E)
 */
int bfsPathExists(Graph *g, int src, int dest)
{
    int visited[MAX_NODES] = {0};
    int queue[MAX_NODES];
    int front = 0, rear = 0;

    visited[src] = 1;
    queue[rear++] = src;

    while (front < rear)
    {
        int curr = queue[front++];
        if (curr == dest)
            return 1;

        for (Edge *e = g->nodes[curr].head; e; e = e->next)
        {
            if (!visited[e->dest] && !e->blocked)
            {
                visited[e->dest] = 1;
                queue[rear++] = e->dest;
            }
        }
    }
    return 0; // No path found
}