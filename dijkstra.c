#include <stdio.h>
#include <string.h>
#include "dijkstra.h"

/*
 * DIJKSTRA'S ALGORITHM
 * --------------------
 * Finds the single-source shortest path in a weighted graph.
 *
 * Why Dijkstra for evacuation routing?
 *   - Roads have different distances/congestion weights
 *   - Guarantees the OPTIMAL (shortest/cheapest) route to shelter
 *   - Works on any weighted graph with non-negative edges
 *
 * Time Complexity: O((V + E) log V) with a min-heap
 *   - V = nodes (locations), E = edges (roads)
 *   - Far better than Bellman-Ford's O(VE) for dense road maps
 *
 * We implement a simple min-heap (priority queue) manually in C.
 */

// ---- Min-Heap (Priority Queue) ----
typedef struct
{
    int node, dist;
} HeapNode;

typedef struct
{
    HeapNode data[MAX_NODES * MAX_NODES];
    int size;
} MinHeap;

static void swap(HeapNode *a, HeapNode *b)
{
    HeapNode tmp = *a;
    *a = *b;
    *b = tmp;
}

static void heapifyUp(MinHeap *h, int i)
{
    while (i > 0)
    {
        int parent = (i - 1) / 2;
        if (h->data[parent].dist > h->data[i].dist)
        {
            swap(&h->data[parent], &h->data[i]);
            i = parent;
        }
        else
            break;
    }
}

static void heapifyDown(MinHeap *h, int i)
{
    while (1)
    {
        int smallest = i, l = 2 * i + 1, r = 2 * i + 2;
        if (l < h->size && h->data[l].dist < h->data[smallest].dist)
            smallest = l;
        if (r < h->size && h->data[r].dist < h->data[smallest].dist)
            smallest = r;
        if (smallest == i)
            break;
        swap(&h->data[i], &h->data[smallest]);
        i = smallest;
    }
}

static void push(MinHeap *h, int node, int dist)
{
    h->data[h->size].node = node;
    h->data[h->size].dist = dist;
    heapifyUp(h, h->size++);
}

static HeapNode pop(MinHeap *h)
{
    HeapNode top = h->data[0];
    h->data[0] = h->data[--h->size];
    heapifyDown(h, 0);
    return top;
}

// ---- Dijkstra's Core ----
int dijkstra(Graph *g, int src, int dest, int *path, int *pathLen)
{
    int dist[MAX_NODES], prev[MAX_NODES], visited[MAX_NODES];

    for (int i = 0; i < g->count; i++)
    {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }

    dist[src] = 0;
    MinHeap heap = {.size = 0};
    push(&heap, src, 0);

    while (heap.size > 0)
    {
        HeapNode curr = pop(&heap);
        int u = curr.node;

        if (visited[u])
            continue;
        visited[u] = 1;

        if (u == dest)
            break; // Found destination early

        for (Edge *e = g->nodes[u].head; e; e = e->next)
        {
            int v = e->dest;
            if (e->blocked || visited[v])
                continue;

            int newDist = dist[u] + e->weight;
            if (newDist < dist[v])
            {
                dist[v] = newDist;
                prev[v] = u;
                push(&heap, v, newDist);
            }
        }
    }

    // Reconstruct path
    *pathLen = 0;
    if (dist[dest] == INF)
    {
        printf("[Dijkstra] No path found from '%s' to '%s'!\n",
               g->nodes[src].name, g->nodes[dest].name);
        return INF;
    }

    // Trace back from dest to src
    int tmp[MAX_NODES], len = 0, cur = dest;
    while (cur != -1)
    {
        tmp[len++] = cur;
        cur = prev[cur];
    }

    // Reverse
    for (int i = 0; i < len; i++)
        path[i] = tmp[len - 1 - i];
    *pathLen = len;

    printf("\n[Dijkstra] Shortest evacuation route:\n");
    printf("  ");
    for (int i = 0; i < len; i++)
    {
        printf("%s", g->nodes[path[i]].name);
        if (i < len - 1)
            printf(" → ");
    }
    printf("\n  Total distance: %d units\n", dist[dest]);

    return dist[dest];
}