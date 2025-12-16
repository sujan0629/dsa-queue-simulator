# DSA Queue Simulator - Traffic Light Management System

## Features
- Queue-based vehicle management
- Priority lane handling (AL2)
- Traffic light simulation
- Socket communication
- SDL graphics rendering
- Comprehensive logging and testing

## Author
Sujan Bhatta - Roll No. 14 (CS II/I)

## Status
🚧 Under Development

## How to Run

### Prerequisites
- C compiler (e.g., gcc)
- Make utility

### Build
```bash
make
```

### Run
Run the traffic generator in the background:
```bash
./traffic_generator &
```

Then run the simulator:
```bash
./simulator
```

The simulator will poll lane files every 5 seconds, process vehicles, and display status. Use Ctrl+C to stop.

### Advanced Usage
- Run multiple generators: `./traffic_generator & ./traffic_generator2 & ./traffic_generator3 &`
- Monitor with receivers: `./reciever & ./reciever2 &`
- Test queue: `./test_queue`
- View log: `cat simulation_log.txt`

## References
- Assignment: COMP202 DSA Queue Simulator
- SDL: https://wiki.libsdl.org/SDL2/FrontPage
- GitHub: https://github.com/sujan0629/dsa-queue-simulator