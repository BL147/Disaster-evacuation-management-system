CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm

TARGET = evacuation

SRCS = main.c graph.c bfs.c dijkstra.c astar.c kruskal.c reroute.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

run: all
	./$(TARGET)