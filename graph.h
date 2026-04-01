#ifndef GRAPH_H
#define GRAPH_H

#define MAX_NODES 100
#define INF 99999

extern int QUIET_MODE;
#define LOG(...)                 \
    do                           \
    {                            \
        if (!QUIET_MODE)         \
            printf(__VA_ARGS__); \
    } while (0)

// ---------- Node types ----------
typedef enum
{
    ZONE,    // Residential/danger zone (evacuation source)
    SHELTER, // Safe destination
    JUNCTION // Road intersection
} NodeType;

// ---------- Adjacency List Edge ----------
typedef struct Edge
{
    int dest;    // Destination node index
    int weight;  // Road distance/cost
    int blocked; // 1 = road closed, 0 = open
    struct Edge *next;
} Edge;

// ---------- Graph Node ----------
typedef struct
{
    char name[50]; // e.g., "Zone A", "Shelter 1"
    NodeType type;
    int x, y;   // Coordinates for A* heuristic
    Edge *head; // Adjacency list head
} Node;

// ---------- Graph ----------
typedef struct
{
    Node nodes[MAX_NODES];
    int count; // Total number of nodes
} Graph;

// ---------- Function Declarations ----------
Graph *createGraph();
int addNode(Graph *g, const char *name, NodeType type, int x, int y);
void addEdge(Graph *g, int src, int dest, int weight);
void blockEdge(Graph *g, int src, int dest); // Road closure
void unblockEdge(Graph *g, int src, int dest);
void printGraph(Graph *g);
void freeGraph(Graph *g);

#endif