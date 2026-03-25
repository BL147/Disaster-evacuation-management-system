# Evacuation Project

C language graph-based pathfinding project with BFS, Dijkstra, A*, Kruskal and reroute logic.

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
