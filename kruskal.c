#include <stdio.h>
#include <stdlib.h>
#include "kruskal.h"

/*
 * KRUSKAL'S ALGORITHM (Minimum Spanning Tree)
 * --------------------------------------------
 * Finds the MINIMUM set of roads that keeps the entire city connected.
 *
 * Why MST for evacuation planning?
 *   - Emergency planners need to know: "Which roads are critical?"
 *   - MST = the backbone road network with minimum total cost
 *   - If you had to build/maintain only N-1 roads, MST tells you which ones
 *   - Useful for pre-disaster infrastructure planning
 *
 * Algorithm:
 *   1. Sort ALL edges by weight (ascending)
 *   2. Pick each edge — add it if it doesn't form a cycle
 *   3. Stop when we have V-1 edges (spanning tree complete)
 *
 * Cycle detection: Union-Find (Disjoint Set Union) — O(α(V)) per operation
 *
 * Time Complexity: O(E log E) — dominated by sorting
 *   - Better than Prim's O(E log V) with adjacency list for sparse graphs
 *   - Prim's is better for dense graphs; Kruskal's wins here (city roads = sparse)
 */

// ---- Union-Find (DSU) ----
static int parent[MAX_NODES], rank_[MAX_NODES];

static void makeSet(int n)
{
    for (int i = 0; i < n; i++)
    {
        parent[i] = i;
        rank_[i] = 0;
    }
}

// Path compression
static int find(int x)
{
    if (parent[x] != x)
        parent[x] = find(parent[x]);
    return parent[x];
}

// Union by rank
static void unite(int a, int b)
{
    int ra = find(a), rb = find(b);
    if (ra == rb)
        return;
    if (rank_[ra] < rank_[rb])
    {
        int t = ra;
        ra = rb;
        rb = t;
    }
    parent[rb] = ra;
    if (rank_[ra] == rank_[rb])
        rank_[ra]++;
}

// ---- Edge list for Kruskal ----
typedef struct
{
    int src, dest, weight;
} KEdge;

static int cmpEdge(const void *a, const void *b)
{
    return ((KEdge *)a)->weight - ((KEdge *)b)->weight;
}

int kruskalMST(Graph *g)
{
    // Collect all unique edges (undirected: only src < dest)
    KEdge edges[MAX_NODES * MAX_NODES];
    int eCount = 0;

    for (int u = 0; u < g->count; u++)
    {
        for (Edge *e = g->nodes[u].head; e; e = e->next)
        {
            if (u < e->dest && !e->blocked)
            {
                edges[eCount].src = u;
                edges[eCount].dest = e->dest;
                edges[eCount].weight = e->weight;
                eCount++;
            }
        }
    }

    // Sort edges by weight
    qsort(edges, eCount, sizeof(KEdge), cmpEdge);

    makeSet(g->count);

    printf("\n[Kruskal's MST] Minimum road network to connect all locations:\n");
    printf("  %-20s %-20s  Cost\n", "From", "To");
    printf("  %-20s %-20s  ----\n", "----", "--");

    int totalWeight = 0, edgesUsed = 0;

    for (int i = 0; i < eCount && edgesUsed < g->count - 1; i++)
    {
        int u = edges[i].src, v = edges[i].dest;

        if (find(u) != find(v))
        {
            unite(u, v);
            printf("  %-20s %-20s  %d\n",
                   g->nodes[u].name, g->nodes[v].name, edges[i].weight);
            totalWeight += edges[i].weight;
            edgesUsed++;
        }
    }

    printf("\n  MST Total Cost : %d units\n", totalWeight);
    printf("  Roads used     : %d (minimum to connect %d locations)\n",
           edgesUsed, g->count);

    return totalWeight;
}