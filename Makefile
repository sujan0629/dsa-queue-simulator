CC = gcc
CFLAGS = -I src -Wall -Wextra

all: simulator traffic_generator reciever

simulator: src/simulator.c src/queue.c
	$(CC) $(CFLAGS) -o simulator src/simulator.c src/queue.c

traffic_generator: src/traffic_generator.c
	$(CC) $(CFLAGS) -o traffic_generator src/traffic_generator.c

reciever: src/reciever.c
	$(CC) $(CFLAGS) -o reciever src/reciever.c

clean:
	rm -f simulator traffic_generator reciever
