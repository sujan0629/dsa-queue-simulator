#!/bin/bash
# Final test script

echo "Building all..."
make

echo "Running tests..."
./test_queue
./test_integration

echo "Tests completed. Check simulation_log.txt for logs."