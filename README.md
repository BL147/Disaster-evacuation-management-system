# Evacuation Project

C language graph-based pathfinding project with BFS, Dijkstra, A\*, Kruskal and reroute logic.

## Prerequisites

- macOS (or Linux)
- `gcc` compiler
- `make`

## Build

```bash
make
```

This compiles:

- `main.c`
- `graph.c`
- `bfs.c`
- `dijkstra.c`
- `astar.c`
- `kruskal.c`
- `reroute.c`

Output binary: `evacuation`

## Run

```bash
make run
```

or

```bash
./evacuation
```

If the executable accepts command line arguments, pass them after `./evacuation`.

## Clean

```bash
make clean
```

## Notes

- `CFLAGS = -Wall -Wextra -g`
- `LDFLAGS = -lm`
- If you get errors, ensure you're in the project root and `gcc` is installed.

## Web App

A web dashboard has been added under `web/` that connects to the C evacuation binary through a light JSON API.

Run the server:

```bash
cd web
node server.js
```

Then open `http://localhost:3000`.

The web UI supports:

- Graph layout and node rendering by coordinate
- BFS danger radius highlighting
- Dijkstra and A\* shortest-path visualization
- Algorithm comparison output
- Kruskal MST backbone display
- Dynamic reroute simulation after a road blockage

Why each algorithm is useful:

- BFS is used for danger-zone expansion because it explores the graph level-by-level.
- Dijkstra gives the optimal evacuation route on weighted roads.
- A\* uses coordinates to guide the search and often explores fewer nodes than Dijkstra.
- Kruskal identifies the minimum road backbone needed to keep the city connected.
- Rerouting simulates real-time road failures and finds alternate safe routes.
