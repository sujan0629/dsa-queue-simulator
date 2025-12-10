# DSA Queue Simulator - Traffic Light Management System

## Project Overview
A traffic junction simulator using queue data structures to manage vehicle flow across 4 roads with priority lane handling.

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

### Clean
```bash
make clean
```

## References
Coming soon...