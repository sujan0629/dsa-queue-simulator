#!/bin/bash
# Demo script to run the simulation

echo "Starting DSA Queue Simulator Demo"

# Build all
make

# Run in background
./traffic_generator &
./reciever &
./simulator

# Cleanup
make clean