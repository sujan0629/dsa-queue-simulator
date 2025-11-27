CC = gcc
CFLAGS = -I src -Wall -Wextra

all: simulator traffic_generator

simulator: src/simulator.c src/queue.c
	$(CC) $(CFLAGS) -o simulator src/simulator.c src/queue.c

traffic_generator: src/traffic_generator.c
	$(CC) $(CFLAGS) -o traffic_generator src/traffic_generator.c

clean:
	rm -f simulator traffic_generator
