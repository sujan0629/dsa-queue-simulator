CC = gcc
# Use pkg-config but remove pkg-config's automatic -Dmain=SDL_main
# which breaks code that defines main normally (we use SDL_MAIN_HANDLED).
CFLAGS = -I src -Wall -Wextra $(shell pkg-config --cflags sdl2 | sed "s/-Dmain=SDL_main//g")
LDFLAGS =
LDFLAGS_SDL = $(shell pkg-config --libs sdl2)

# On Windows (MSYS/MinGW) link with Winsock library
ifeq ($(OS),Windows_NT)
	LDFLAGS += -lws2_32
endif

all: simulator traffic_generator reciever traffic_generator2 traffic_generator3 reciever2 test_queue test_integration graphics

simulator: src/simulator.c src/queue.c
	$(CC) $(CFLAGS) -o simulator src/simulator.c src/queue.c $(LDFLAGS)

traffic_generator: src/traffic_generator.c
	$(CC) $(CFLAGS) -o traffic_generator src/traffic_generator.c $(LDFLAGS)

reciever: src/reciever.c
	$(CC) $(CFLAGS) -o reciever src/reciever.c $(LDFLAGS)

traffic_generator2: src/traffic_generator2.c
	$(CC) $(CFLAGS) -o traffic_generator2 src/traffic_generator2.c $(LDFLAGS)

traffic_generator3: src/traffic_generator3.c
	$(CC) $(CFLAGS) -o traffic_generator3 src/traffic_generator3.c $(LDFLAGS)

reciever2: src/reciever2.c
	$(CC) $(CFLAGS) -o reciever2 src/reciever2.c $(LDFLAGS)

test_integration: src/test_integration.c src/queue.c
	$(CC) $(CFLAGS) -o test_integration src/test_integration.c src/queue.c $(LDFLAGS)

graphics: src/graphics.c
	$(CC) $(CFLAGS) -o graphics src/graphics.c $(LDFLAGS_SDL)

clean:
	rm -f simulator traffic_generator reciever traffic_generator2 traffic_generator3 reciever2 test_queue test_integration graphics
