#!/bin/bash
# Demo script to run the simulation

echo "Starting DSA Queue Simulator Demo"

# Build all
make

# Run simulator in background
./simulator &
SIM_PID=$!

# Wait a bit
sleep 2

# Run generator
./traffic_generator

# Kill simulator
kill $SIM_PID

# Cleanup
make clean