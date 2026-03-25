#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "bfs.h"
#include "dijkstra.h"
#include "astar.h"
#include "kruskal.h"
#include "reroute.h"

/*
 * DISASTER EVACUATION ROUTE OPTIMISATION
 * =======================================
 * Sample city map (12 nodes):
 *
 *  [0] Zone A ──10── [4] Junction 1 ──15── [8]  Shelter North
 *      |                   |                         |
 *     20                  12                        10
 *      |                   |                         |
 *  [1] Zone B ──8───  [5] Junction 2 ──20── [9]  Shelter East
 *      |                   |
 *     15                  18
 *      |                   |
 *  [2] Zone C ──25── [6] Junction 3 ──30── [10] Shelter South
 *      |                   |
 *     10                  22
 *      |                   |
 *  [3] Zone D ──14── [7] Junction 4 ──16── [11] Shelter West
 *
 * Zones are danger areas (evacuation sources)
 * Shelters are safe destinations
 * Junctions are intermediate road intersections
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
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
}

void compareAlgorithms(Graph *g, int src, int dest)
{
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║  ALGORITHM COMPARISON: %s → %s\n", g->nodes[src].name, g->nodes[dest].name);
    printf("╚══════════════════════════════════════════╝\n");

    int path[MAX_NODES], pathLen;

    printf("\n--- Dijkstra ---");
    int dDist = dijkstra(g, src, dest, path, &pathLen);

    printDivider();

    printf("--- A* Search ---");
    int aDist = astar(g, src, dest, path, &pathLen);

    printDivider();

    printf("[Summary]\n");
    printf("  Dijkstra distance : %d\n", dDist);
    printf("  A* distance       : %d\n", aDist);
    printf("  Both optimal?     : %s\n", (dDist == aDist) ? "YES ✓" : "NO — check heuristic");
    printf("  A* advantage      : Fewer nodes explored (guided by coordinates)\n");
}

int main()
{
    Graph *g = buildSampleCity();

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   DISASTER EVACUATION ROUTE OPTIMISATION     ║\n");
    printf("║   DSA Project — Algorithm Showcase           ║\n");
    printf("╚══════════════════════════════════════════════╝\n");

    int choice;
    do
    {
        printf("\n===== MAIN MENU =====\n");
        printf("1. View city road network\n");
        printf("2. BFS — Identify danger zones from disaster origin\n");
        printf("3. Dijkstra — Find shortest evacuation route\n");
        printf("4. A* Search — Find optimal route (heuristic-guided)\n");
        printf("5. Compare Dijkstra vs A*\n");
        printf("6. Kruskal's MST — Minimum road backbone\n");
        printf("7. Dynamic Rerouting — Simulate road blockage\n");
        printf("0. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        int src, dest, path[MAX_NODES], pathLen;

        switch (choice)
        {
        case 1:
            printGraph(g);
            break;

        case 2:
        {
            printf("Enter disaster origin node index (0=Zone A, 1=Zone B, 2=Zone C, 3=Zone D): ");
            scanf("%d", &src);
            int radius;
            printf("Enter danger radius (hops, e.g. 2): ");
            scanf("%d", &radius);
            int affected[MAX_NODES];
            bfsFloodZones(g, src, radius, affected);
            break;
        }

        case 3:
            printf("Enter source node index: ");
            scanf("%d", &src);
            printf("Enter shelter node index (8=North, 9=East, 10=South, 11=West): ");
            scanf("%d", &dest);
            dijkstra(g, src, dest, path, &pathLen);
            break;

        case 4:
            printf("Enter source node index: ");
            scanf("%d", &src);
            printf("Enter shelter node index: ");
            scanf("%d", &dest);
            astar(g, src, dest, path, &pathLen);
            break;

        case 5:
            printf("Enter source node index: ");
            scanf("%d", &src);
            printf("Enter shelter node index: ");
            scanf("%d", &dest);
            compareAlgorithms(g, src, dest);
            break;

        case 6:
            kruskalMST(g);
            break;

        case 7:
        {
            int rSrc, rDest;
            printf("Enter road to block (src dest): ");
            scanf("%d %d", &rSrc, &rDest);
            printf("Enter evacuee node index: ");
            scanf("%d", &src);
            printf("Enter shelter node index: ");
            scanf("%d", &dest);
            rerouteAfterBlockage(g, rSrc, rDest, src, dest);
            break;
        }

        case 0:
            printf("Exiting. Stay safe!\n");
            break;

        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 0);

    freeGraph(g);
    return 0;
}