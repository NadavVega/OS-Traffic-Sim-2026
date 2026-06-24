CC = gcc
CFLAGS = -Isrc -Wall
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
COMMON_SOURCES = src/main.c src/child.c src/dijkstra.c src/gui.c src/parser.c src/graph.c
MS5_SOURCES = $(COMMON_SOURCES) src/ipc.c
MS6_SOURCES = $(MS5_SOURCES) src/node_locks.c
MS7_SOURCES = $(MS5_SOURCES) src/scheduler.c

.PHONY: milestone1 milestone2 milestone3 milestone4 milestone5 milestone6 milestone7 clean

# Milestone 1: Requirement is an executable named 'dijkstra'
milestone1: $(COMMON_SOURCES)
	$(CC) $(CFLAGS) -DMILESTONE=4 $(COMMON_SOURCES) -o dijkstra $(LIBS)

# Milestone 4: Requirement is an executable named 'sim'
milestone2: milestone3
milestone3: milestone4
milestone4: $(COMMON_SOURCES)
	$(CC) $(CFLAGS) -DMILESTONE=4 $(COMMON_SOURCES) -o sim $(LIBS)

# Milestone 5: children calculate routes and report progress over IPC.
milestone5: $(MS5_SOURCES)
	$(CC) $(CFLAGS) -DMILESTONE=5 $(MS5_SOURCES) -o sim $(LIBS)

# Milestone 6: node locking uses System V semaphores.
milestone6: $(MS6_SOURCES)
	$(CC) $(CFLAGS) -DMILESTONE=6 $(MS6_SOURCES) -o sim $(LIBS)

# Milestone 7: parent scheduler grants node access.
milestone7: $(MS7_SOURCES)
	$(CC) $(CFLAGS) -DMILESTONE=7 $(MS7_SOURCES) -o sim $(LIBS)

clean:
	rm -f dijkstra sim *.o src/*.o
