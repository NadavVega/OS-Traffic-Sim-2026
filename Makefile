CC = gcc
CFLAGS = -Isrc -Wall
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Milestone 1: Requirement is an executable named 'dijkstra'
milestone1: src/main.c src/dijkstra.c src/gui.c src/parser.c src/graph.c
	$(CC) $(CFLAGS) src/main.c src/dijkstra.c src/gui.c src/parser.c src/graph.c -o dijkstra $(LIBS)

# Milestone 4: Requirement is an executable named 'sim'
milestone2: milestone3
milestone3: milestone4
milestone4: src/main.c src/dijkstra.c src/gui.c src/parser.c src/graph.c    
	$(CC) $(CFLAGS) src/main.c src/dijkstra.c src/gui.c src/parser.c src/graph.c -o sim $(LIBS)

clean:
	rm -f dijkstra sim