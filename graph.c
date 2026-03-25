#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

Graph *createGraph()
{
    Graph *g = (Graph *)malloc(sizeof(Graph));
    g->count = 0;
    return g;
}

// Returns the index of the newly added node
int addNode(Graph *g, const char *name, NodeType type, int x, int y)
{
    int i = g->count;
    strncpy(g->nodes[i].name, name, 49);
    g->nodes[i].type = type;
    g->nodes[i].x = x;
    g->nodes[i].y = y;
    g->nodes[i].head = NULL;
    g->count++;
    return i;
}

// Undirected weighted edge (adds both directions)
void addEdge(Graph *g, int src, int dest, int weight)
{
    // src -> dest
    Edge *e1 = (Edge *)malloc(sizeof(Edge));
    e1->dest = dest;
    e1->weight = weight;
    e1->blocked = 0;
    e1->next = g->nodes[src].head;
    g->nodes[src].head = e1;

    // dest -> src (undirected)
    Edge *e2 = (Edge *)malloc(sizeof(Edge));
    e2->dest = src;
    e2->weight = weight;
    e2->blocked = 0;
    e2->next = g->nodes[dest].head;
    g->nodes[dest].head = e2;
}

void blockEdge(Graph *g, int src, int dest)
{
    // Block both directions
    for (Edge *e = g->nodes[src].head; e; e = e->next)
        if (e->dest == dest)
        {
            e->blocked = 1;
            break;
        }
    for (Edge *e = g->nodes[dest].head; e; e = e->next)
        if (e->dest == src)
        {
            e->blocked = 1;
            break;
        }
    printf("[!] Road between '%s' and '%s' is now CLOSED.\n",
           g->nodes[src].name, g->nodes[dest].name);
}

void unblockEdge(Graph *g, int src, int dest)
{
    for (Edge *e = g->nodes[src].head; e; e = e->next)
        if (e->dest == dest)
        {
            e->blocked = 0;
            break;
        }
    for (Edge *e = g->nodes[dest].head; e; e = e->next)
        if (e->dest == src)
        {
            e->blocked = 0;
            break;
        }
    printf("[✓] Road between '%s' and '%s' is now OPEN.\n",
           g->nodes[src].name, g->nodes[dest].name);
}

void printGraph(Graph *g)
{
    const char *types[] = {"ZONE", "SHELTER", "JUNCTION"};
    printf("\n===== City Road Network =====\n");
    for (int i = 0; i < g->count; i++)
    {
        printf("[%d] %s (%s)\n", i, g->nodes[i].name, types[g->nodes[i].type]);
        for (Edge *e = g->nodes[i].head; e; e = e->next)
        {
            printf("     --> %s (dist: %d)%s\n",
                   g->nodes[e->dest].name, e->weight,
                   e->blocked ? " [BLOCKED]" : "");
        }
    }
    printf("=============================\n\n");
}

void freeGraph(Graph *g)
{
    for (int i = 0; i < g->count; i++)
    {
        Edge *e = g->nodes[i].head;
        while (e)
        {
            Edge *tmp = e;
            e = e->next;
            free(tmp);
        }
    }
    free(g);
}