#include <stdio.h>
#include <string.h>
#include <math.h>
#include "astar.h"

/*
 * A* SEARCH ALGORITHM
 * -------------------
 * An informed search that uses a heuristic to guide exploration.
 *
 * f(n) = g(n) + h(n)
 *   g(n) = actual cost from start to node n  (like Dijkstra)
 *   h(n) = estimated cost from n to goal     (heuristic — Euclidean distance)
 *
 * Why A* over Dijkstra for evacuation?
 *   - Dijkstra explores ALL directions equally (blind)
 *   - A* focuses toward the shelter using GPS coordinates
 *   - In large city maps, A* skips irrelevant roads entirely
 *   - Explores FEWER nodes → faster in practice
 *
 * Time Complexity: O(E log V) in best case (good heuristic)
 *   - Worst case same as Dijkstra: O((V+E) log V)
 *   - Heuristic NEVER overestimates → result is always optimal (admissible)
 *
 * Why Euclidean heuristic?
 *   - Nodes have (x, y) coordinates (like a city map)
 *   - Straight-line distance is always ≤ actual road distance
 *   - This makes it "admissible" → A* stays optimal
 */

// Euclidean distance between two nodes (heuristic)
static double heuristic(Graph *g, int a, int b)
{
    int dx = g->nodes[a].x - g->nodes[b].x;
    int dy = g->nodes[a].y - g->nodes[b].y;
    return sqrt((double)(dx * dx + dy * dy));
}

// ---- Min-Heap for A* (stores f-score) ----
typedef struct
{
    int node;
    double f;
} AStarNode;

typedef struct
{
    AStarNode data[MAX_NODES * MAX_NODES];
    int size;
} AStarHeap;

static void aswap(AStarNode *a, AStarNode *b)
{
    AStarNode t = *a;
    *a = *b;
    *b = t;
}

static void asHeapifyUp(AStarHeap *h, int i)
{
    while (i > 0)
    {
        int p = (i - 1) / 2;
        if (h->data[p].f > h->data[i].f)
        {
            aswap(&h->data[p], &h->data[i]);
            i = p;
        }
        else
            break;
    }
}

static void asHeapifyDown(AStarHeap *h, int i)
{
    while (1)
    {
        int s = i, l = 2 * i + 1, r = 2 * i + 2;
        if (l < h->size && h->data[l].f < h->data[s].f)
            s = l;
        if (r < h->size && h->data[r].f < h->data[s].f)
            s = r;
        if (s == i)
            break;
        aswap(&h->data[i], &h->data[s]);
        i = s;
    }
}

static void asPush(AStarHeap *h, int node, double f)
{
    h->data[h->size].node = node;
    h->data[h->size].f = f;
    asHeapifyUp(h, h->size++);
}

static AStarNode asPop(AStarHeap *h)
{
    AStarNode top = h->data[0];
    h->data[0] = h->data[--h->size];
    asHeapifyDown(h, 0);
    return top;
}

// ---- A* Core ----
int astar(Graph *g, int src, int dest, int *path, int *pathLen)
{
    double gScore[MAX_NODES];
    int prev[MAX_NODES], visited[MAX_NODES];

    for (int i = 0; i < g->count; i++)
    {
        gScore[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }

    gScore[src] = 0;
    AStarHeap heap = {.size = 0};
    asPush(&heap, src, heuristic(g, src, dest));

    int nodesExplored = 0;

    while (heap.size > 0)
    {
        AStarNode curr = asPop(&heap);
        int u = curr.node;

        if (visited[u])
            continue;
        visited[u] = 1;
        nodesExplored++;

        if (u == dest)
            break;

        for (Edge *e = g->nodes[u].head; e; e = e->next)
        {
            int v = e->dest;
            if (e->blocked || visited[v])
                continue;

            double tentativeG = gScore[u] + e->weight;
            if (tentativeG < gScore[v])
            {
                gScore[v] = tentativeG;
                prev[v] = u;
                double f = tentativeG + heuristic(g, v, dest);
                asPush(&heap, v, f);
            }
        }
    }

    // Reconstruct path
    *pathLen = 0;
    if (gScore[dest] >= INF)
    {
        printf("[A*] No path found from '%s' to '%s'!\n",
               g->nodes[src].name, g->nodes[dest].name);
        return INF;
    }

    int tmp[MAX_NODES], len = 0, cur = dest;
    while (cur != -1)
    {
        tmp[len++] = cur;
        cur = prev[cur];
    }
    for (int i = 0; i < len; i++)
        path[i] = tmp[len - 1 - i];
    *pathLen = len;

    printf("\n[A*] Optimal evacuation route (heuristic-guided):\n");
    printf("  ");
    for (int i = 0; i < len; i++)
    {
        printf("%s", g->nodes[path[i]].name);
        if (i < len - 1)
            printf(" → ");
    }
    printf("\n  Total distance : %d units\n", (int)gScore[dest]);
    printf("  Nodes explored : %d (vs Dijkstra explores all reachable)\n", nodesExplored);

    return (int)gScore[dest];
}