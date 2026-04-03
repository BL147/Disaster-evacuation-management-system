#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "bfs.h"
#include "dijkstra.h"
#include "astar.h"
#include "kruskal.h"
#include "reroute.h"

int QUIET_MODE = 0;

static const char *nodeTypeNames[] = {"ZONE", "SHELTER", "JUNCTION"};

typedef struct
{
    int src;
    int dest;
    int weight;
} KEdge;

static int kruskalParent[MAX_NODES];
static int kruskalRank[MAX_NODES];

static int kruskalFind(int x)
{
    if (kruskalParent[x] != x)
        kruskalParent[x] = kruskalFind(kruskalParent[x]);
    return kruskalParent[x];
}

static void kruskalUnion(int a, int b)
{
    int ra = kruskalFind(a);
    int rb = kruskalFind(b);
    if (ra == rb)
        return;
    if (kruskalRank[ra] < kruskalRank[rb])
        kruskalParent[ra] = rb;
    else
    {
        kruskalParent[rb] = ra;
        if (kruskalRank[ra] == kruskalRank[rb])
            kruskalRank[ra]++;
    }
}

static int compareKEdge(const void *a, const void *b)
{
    const KEdge *ea = a;
    const KEdge *eb = b;
    return ea->weight - eb->weight;
}

static void printGraphJson(Graph *g)
{
    printf("{\"nodes\":[");
    for (int i = 0; i < g->count; i++)
    {
        printf("{\"id\":%d,\"name\":\"%s\",\"type\":\"%s\",\"x\":%d,\"y\":%d}",
               i, g->nodes[i].name, nodeTypeNames[g->nodes[i].type], g->nodes[i].x, g->nodes[i].y);
        if (i < g->count - 1)
            printf(",");
    }
    printf("],\"edges\":[");
    int first = 1;
    for (int u = 0; u < g->count; u++)
    {
        for (Edge *e = g->nodes[u].head; e; e = e->next)
        {
            if (u < e->dest)
            {
                if (!first)
                    printf(",");
                first = 0;
                printf("{\"src\":%d,\"dest\":%d,\"weight\":%d,\"blocked\":%d}",
                       u, e->dest, e->weight, e->blocked);
            }
        }
    }
    printf("]}");
}

static void printBfsJson(Graph *g, int origin, int radius)
{
    int affected[MAX_NODES] = {0};
    QUIET_MODE = 1;
    int count = bfsFloodZones(g, origin, radius, affected);
    QUIET_MODE = 0;

    printf("{\"origin\":%d,\"radius\":%d,\"affected\":[", origin, radius);
    for (int i = 0; i < count; i++)
    {
        printf("%d", affected[i]);
        if (i < count - 1)
            printf(",");
    }
    printf("],\"count\":%d}", count);
}

static void printPathJson(const char *name, Graph *g, int src, int dest, int (*search)(Graph *, int, int, int *, int *))
{
    int path[MAX_NODES], pathLen;
    QUIET_MODE = 1;
    int distance = search(g, src, dest, path, &pathLen);
    QUIET_MODE = 0;

    printf("{\"algorithm\":\"%s\",\"source\":%d,\"destination\":%d,\"distance\":%d,\"path\":[",
           name, src, dest, distance == INF ? -1 : distance);
    for (int i = 0; i < pathLen; i++)
    {
        printf("%d", path[i]);
        if (i < pathLen - 1)
            printf(",");
    }
    printf("]}");
}

static void printCompareJson(Graph *g, int src, int dest)
{
    int path[MAX_NODES], pathLen;
    QUIET_MODE = 1;
    int dDist = dijkstra(g, src, dest, path, &pathLen);
    int aDist = astar(g, src, dest, path, &pathLen);
    QUIET_MODE = 0;

    printf("{\"source\":%d,\"destination\":%d,\"dijkstra\":%d,\"astar\":%d,\"bothOptimal\":%s}",
           src, dest, dDist == INF ? -1 : dDist, aDist == INF ? -1 : aDist,
           (dDist == aDist) ? "true" : "false");
}

static void printKruskalJson(Graph *g)
{
    int eCount = 0;
    KEdge edges[MAX_NODES * MAX_NODES];

    for (int u = 0; u < g->count; u++)
    {
        for (Edge *e = g->nodes[u].head; e; e = e->next)
        {
            if (u < e->dest && !e->blocked)
            {
                edges[eCount++] = (KEdge){u, e->dest, e->weight};
            }
        }
    }

    qsort(edges, eCount, sizeof(KEdge), compareKEdge);

    for (int i = 0; i < g->count; i++)
    {
        kruskalParent[i] = i;
        kruskalRank[i] = 0;
    }

    printf("{\"edges\":[");
    int first = 1;
    int totalWeight = 0;
    int edgesUsed = 0;

    for (int i = 0; i < eCount && edgesUsed < g->count - 1; i++)
    {
        int u = edges[i].src;
        int v = edges[i].dest;
        if (kruskalFind(u) != kruskalFind(v))
        {
            kruskalUnion(u, v);
            if (!first)
                printf(",");
            first = 0;
            printf("{\"src\":%d,\"dest\":%d,\"weight\":%d}", u, v, edges[i].weight);
            totalWeight += edges[i].weight;
            edgesUsed++;
        }
    }

    printf("],\"totalCost\":%d}", totalWeight);
}

static void printRerouteJson(Graph *g, int roadSrc, int roadDest, int evacuee, int shelter)
{
    QUIET_MODE = 1;
    blockEdge(g, roadSrc, roadDest);
    int reachable = bfsPathExists(g, evacuee, shelter);
    int path[MAX_NODES], pathLen;
    int distance = -1;
    int chosenShelter = shelter;
    int alternative = -1;

    if (reachable)
    {
        distance = astar(g, evacuee, shelter, path, &pathLen);
    }
    else
    {
        for (int i = 0; i < g->count; i++)
        {
            if (g->nodes[i].type == SHELTER && i != shelter)
            {
                if (bfsPathExists(g, evacuee, i))
                {
                    alternative = i;
                    distance = astar(g, evacuee, i, path, &pathLen);
                    chosenShelter = i;
                    break;
                }
            }
        }
    }
    QUIET_MODE = 0;

    printf("{\"roadBlocked\":{\"src\":%d,\"dest\":%d},\"evacuee\":%d,\"requestedShelter\":%d,",
           roadSrc, roadDest, evacuee, shelter);

    if (reachable)
    {
        printf("\"reachable\":true,\"shelter\":%d,\"distance\":%d,\"path\":[",
               chosenShelter, distance == INF ? -1 : distance);
        for (int i = 0; i < pathLen; i++)
        {
            printf("%d", path[i]);
            if (i < pathLen - 1)
                printf(",");
        }
        printf("]}");
    }
    else if (alternative != -1)
    {
        printf("\"reachable\":false,\"alternativeShelter\":%d,\"distance\":%d,\"path\":[",
               chosenShelter, distance == INF ? -1 : distance);
        for (int i = 0; i < pathLen; i++)
        {
            printf("%d", path[i]);
            if (i < pathLen - 1)
                printf(",");
        }
        printf("]}");
    }
    else
    {
        printf("\"reachable\":false,\"reachableAnyShelter\":false}");
    }
}

static void printApiUsage(void)
{
    printf("{\"error\":\"Usage: ./evacuation --json <command> [args]\",\"commands\":[\"graph\",\"bfs origin radius\",\"dijkstra src dest\",\"astar src dest\",\"compare src dest\",\"kruskal\",\"reroute roadSrc roadDest evacuee shelter\"]}");
}

static void runApiMode(int argc, char **argv, Graph *g)
{
    if (argc < 3)
    {
        printApiUsage();
        return;
    }

    const char *cmd = argv[2];

    if (strcmp(cmd, "graph") == 0)
    {
        printGraphJson(g);
        return;
    }
    if (strcmp(cmd, "bfs") == 0)
    {
        if (argc != 5)
        {
            printApiUsage();
            return;
        }
        int origin = atoi(argv[3]);
        int radius = atoi(argv[4]);
        printBfsJson(g, origin, radius);
        return;
    }
    if (strcmp(cmd, "dijkstra") == 0)
    {
        if (argc != 5)
        {
            printApiUsage();
            return;
        }
        int src = atoi(argv[3]);
        int dest = atoi(argv[4]);
        printPathJson("Dijkstra", g, src, dest, dijkstra);
        return;
    }
    if (strcmp(cmd, "astar") == 0)
    {
        if (argc != 5)
        {
            printApiUsage();
            return;
        }
        int src = atoi(argv[3]);
        int dest = atoi(argv[4]);
        printPathJson("A*", g, src, dest, astar);
        return;
    }
    if (strcmp(cmd, "compare") == 0)
    {
        if (argc != 5)
        {
            printApiUsage();
            return;
        }
        int src = atoi(argv[3]);
        int dest = atoi(argv[4]);
        printCompareJson(g, src, dest);
        return;
    }
    if (strcmp(cmd, "kruskal") == 0)
    {
        printKruskalJson(g);
        return;
    }
    if (strcmp(cmd, "reroute") == 0)
    {
        if (argc != 7)
        {
            printApiUsage();
            return;
        }
        int roadSrc = atoi(argv[3]);
        int roadDest = atoi(argv[4]);
        int evacuee = atoi(argv[5]);
        int shelter = atoi(argv[6]);
        printRerouteJson(g, roadSrc, roadDest, evacuee, shelter);
        return;
    }

    printApiUsage();
}

/*
 * DISASTER EVACUATION ROUTE OPTIMISATION
 * =======================================
 */

Graph *buildSampleCity()
{
    Graph *g = createGraph();

    // Zones (danger areas) — node indices 0-3
    addNode(g, "Zone A", ZONE, 0, 80);
    addNode(g, "Zone B", ZONE, 0, 50);
    addNode(g, "Zone C", ZONE, 0, 20);
    addNode(g, "Zone D", ZONE, 0, 0);

    // Junctions — node indices 4-7
    addNode(g, "Junction 1", JUNCTION, 40, 80);
    addNode(g, "Junction 2", JUNCTION, 40, 50);
    addNode(g, "Junction 3", JUNCTION, 40, 20);
    addNode(g, "Junction 4", JUNCTION, 40, 0);

    // Shelters — node indices 8-11
    addNode(g, "Shelter North", SHELTER, 80, 100);
    addNode(g, "Shelter East", SHELTER, 80, 50);
    addNode(g, "Shelter South", SHELTER, 80, 10);
    addNode(g, "Shelter West", SHELTER, 80, 0);

    // Zone → Junction roads
    addEdge(g, 0, 4, 10); // Zone A  → Junction 1
    addEdge(g, 1, 4, 20); // Zone B  → Junction 1
    addEdge(g, 1, 5, 8);  // Zone B  → Junction 2
    addEdge(g, 2, 5, 15); // Zone C  → Junction 2
    addEdge(g, 2, 6, 25); // Zone C  → Junction 3
    addEdge(g, 3, 6, 10); // Zone D  → Junction 3
    addEdge(g, 3, 7, 14); // Zone D  → Junction 4

    // Junction → Junction roads (internal)
    addEdge(g, 4, 5, 12); // Junction 1 ↔ Junction 2
    addEdge(g, 5, 6, 18); // Junction 2 ↔ Junction 3
    addEdge(g, 6, 7, 22); // Junction 3 ↔ Junction 4

    // Junction → Shelter roads
    addEdge(g, 4, 8, 15);  // Junction 1 → Shelter North
    addEdge(g, 5, 8, 30);  // Junction 2 → Shelter North
    addEdge(g, 5, 9, 20);  // Junction 2 → Shelter East
    addEdge(g, 6, 10, 30); // Junction 3 → Shelter South
    addEdge(g, 7, 10, 16); // Junction 4 → Shelter South
    addEdge(g, 7, 11, 12); // Junction 4 → Shelter West

    return g;
}

void printDivider()
{
    LOG("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
}

void compareAlgorithms(Graph *g, int src, int dest)
{
    LOG("\n╔══════════════════════════════════════════╗\n");
    LOG("║  ALGORITHM COMPARISON: %s → %s\n", g->nodes[src].name, g->nodes[dest].name);
    LOG("╚══════════════════════════════════════════╝\n");

    int path[MAX_NODES], pathLen;

    LOG("\n--- Dijkstra ---");
    int dDist = dijkstra(g, src, dest, path, &pathLen);

    printDivider();

    LOG("--- A* Search ---");
    int aDist = astar(g, src, dest, path, &pathLen);

    printDivider();

    LOG("[Summary]\n");
    LOG("  Dijkstra distance : %d\n", dDist);
    LOG("  A* distance       : %d\n", aDist);
    LOG("  Both optimal?     : %s\n", (dDist == aDist) ? "YES ✓" : "NO — check heuristic");
    LOG("  A* advantage      : Fewer nodes explored (guided by coordinates)\n");
}

int main(int argc, char **argv)
{
    Graph *g = buildSampleCity();

    if (argc > 1 && strcmp(argv[1], "--json") == 0)
    {
        runApiMode(argc, argv, g);
        freeGraph(g);
        return 0;
    }

    LOG("╔══════════════════════════════════════════════╗\n");
    LOG("║   DISASTER EVACUATION ROUTE OPTIMISATION     ║\n");
    LOG("║   DSA Project — Algorithm Showcase           ║\n");
    LOG("╚══════════════════════════════════════════════╝\n");

    int choice;
    do
    {
        LOG("\n===== MAIN MENU =====\n");
        LOG("1. View city road network\n");
        LOG("2. BFS — Identify danger zones from disaster origin\n");
        LOG("3. Dijkstra — Find shortest evacuation route\n");
        LOG("4. A* Search — Find optimal route (heuristic-guided)\n");
        LOG("5. Compare Dijkstra vs A*\n");
        LOG("6. Kruskal's MST — Minimum road backbone\n");
        LOG("7. Dynamic Rerouting — Simulate road blockage\n");
        LOG("0. Exit\n");
        LOG("Choice: ");
        scanf("%d", &choice);

        int src, dest, path[MAX_NODES], pathLen;

        switch (choice)
        {
        case 1:
            printGraph(g);
            break;

        case 2:
        {
            LOG("Enter disaster origin node index (0=Zone A, 1=Zone B, 2=Zone C, 3=Zone D): ");
            scanf("%d", &src);
            int radius;
            LOG("Enter danger radius (hops, e.g. 2): ");
            scanf("%d", &radius);
            int affected[MAX_NODES];
            bfsFloodZones(g, src, radius, affected);
            break;
        }

        case 3:
            LOG("Enter source node index: ");
            scanf("%d", &src);
            LOG("Enter shelter node index (8=North, 9=East, 10=South, 11=West): ");
            scanf("%d", &dest);
            dijkstra(g, src, dest, path, &pathLen);
            break;

        case 4:
            LOG("Enter source node index: ");
            scanf("%d", &src);
            LOG("Enter shelter node index: ");
            scanf("%d", &dest);
            astar(g, src, dest, path, &pathLen);
            break;

        case 5:
            LOG("Enter source node index: ");
            scanf("%d", &src);
            LOG("Enter shelter node index: ");
            scanf("%d", &dest);
            compareAlgorithms(g, src, dest);
            break;

        case 6:
            kruskalMST(g);
            break;

        case 7:
        {
            int rSrc, rDest;
            LOG("Enter road to block (src dest): ");
            scanf("%d %d", &rSrc, &rDest);
            LOG("Enter evacuee node index: ");
            scanf("%d", &src);
            LOG("Enter shelter node index: ");
            scanf("%d", &dest);
            rerouteAfterBlockage(g, rSrc, rDest, src, dest);
            break;
        }

        case 0:
            LOG("Exiting. Stay safe!\n");
            break;

        default:
            LOG("Invalid choice.\n");
        }
    } while (choice != 0);

    freeGraph(g);
    return 0;
}