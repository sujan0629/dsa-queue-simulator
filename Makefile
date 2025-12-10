CC = gcc
CFLAGS = -I src -Wall -Wextra

all: simulator traffic_generator reciever traffic_generator2 traffic_generator3 reciever2 test_queue graphics

simulator: src/simulator.c src/queue.c
	$(CC) $(CFLAGS) -o simulator src/simulator.c src/queue.c

traffic_generator: src/traffic_generator.c
	$(CC) $(CFLAGS) -o traffic_generator src/traffic_generator.c

reciever: src/reciever.c
	$(CC) $(CFLAGS) -o reciever src/reciever.c

traffic_generator2: src/traffic_generator2.c
	$(CC) $(CFLAGS) -o traffic_generator2 src/traffic_generator2.c

traffic_generator3: src/traffic_generator3.c
	$(CC) $(CFLAGS) -o traffic_generator3 src/traffic_generator3.c

reciever2: src/reciever2.c
	$(CC) $(CFLAGS) -o reciever2 src/reciever2.c

test_queue: src/test_queue.c src/queue.c
	$(CC) $(CFLAGS) -o test_queue src/test_queue.c src/queue.c

graphics: src/graphics.c
	$(CC) $(CFLAGS) -o graphics src/graphics.c

clean:
	rm -f simulator traffic_generator reciever traffic_generator2 traffic_generator3 reciever2 test_queue graphics
