# DSA Queue Simulator - Traffic Light Management System

## Introduction

This project implements a traffic light management system using data structures and algorithms learned in COMP202. The simulator models four lanes of traffic (A, B, C, D) with vehicles arriving and being processed based on traffic light cycles. It incorporates priority handling for lane A (AL2), socket-based communication between traffic generators and the simulator, and optional SDL graphics for visualization. The system demonstrates the use of queues for FIFO vehicle management, priority scheduling, and inter-process communication.

## Data Structures Used

The core data structures are implemented in C using linked lists and structs:

| Data Structure | Implementation | Purpose |
|----------------|-----------------|---------|
| Queue | Linked List (Node with Vehicle, front/rear pointers, size) | Store vehicles in each lane in FIFO order. Each queue has operations for enqueue, dequeue, is_empty, and size. |
| Vehicle | Struct with int id | Represent individual vehicles with a unique identifier for tracking. |
| LightState | Enum (RED, GREEN) | Track the current state of the traffic light for each lane. |
| Lane | Struct with Queue and LightState | Encapsulate lane-specific data including vehicle queue and light status. |

The linked list implementation ensures O(1) enqueue and dequeue operations, which is crucial for efficient simulation.

## Algorithm Used

The simulation algorithm follows these steps:

1. **Initialization**: Create four lane queues and initialize traffic lights to RED. Start socket server for communication.

2. **Vehicle Generation**: Traffic generators (clients) connect via TCP sockets and send vehicle data to enqueue in specific lanes.

3. **Simulation Loop**:
   - Poll for new vehicles from generators every second.
   - Cycle traffic lights: GREEN for one lane at a time, RED for others.
   - During GREEN phase:
     - Check for priority condition: If lane A (AL2) has >10 vehicles, activate priority mode.
     - In priority mode: Serve lane A until <5 vehicles remain.
     - In normal mode: Calculate proportional serving: vehicles_per_lane = total_vehicles / num_lanes, then serve round-robin.
   - Dequeue vehicles from active lanes and log processing.

4. **Graphics Update**: If SDL is enabled, render lanes, lights, and vehicles in a graphical window.

5. **Termination**: Close sockets and free memory upon simulation end.

This approach ensures fair distribution while allowing priority overrides for congestion.

## Time Complexity

- **Enqueue/Dequeue**: O(1) per operation due to linked list pointers.
- **Priority Check**: O(1) as it only checks one lane's size.
- **Normal Serving**: O(n) where n=4 lanes, for proportional calculation and round-robin.
- **Overall Simulation**: O(t) where t is the number of time steps, dominated by polling and serving loops.
- **Space Complexity**: O(n) where n is total vehicles, stored in linked lists.

The design prioritizes constant-time operations for core queue functions to handle real-time simulation efficiently.

## Implementation Details

- **Queue Module** (`queue.c`): Provides create_queue, enqueue, dequeue, is_empty, size functions.
- **Simulator** (`simulator.c`): Main loop with socket server, light cycling, priority logic.
- **Traffic Generators** (`traffic_generator*.c`): Clients that generate and send vehicles via sockets.
- **Graphics** (`graphics.c`): SDL-based rendering of lanes, lights, and vehicles.
- **Cross-Platform**: Uses conditional compilation for Windows (Winsock) and Unix sockets.
- **Testing**: Unit tests for queue operations, integration tests for full simulation.

## Testing

- Unit tests verify queue operations (enqueue, dequeue, size).
- Integration tests check socket communication and simulation logic.
- Manual testing validates priority activation, proportional serving, and graphics display.
- All tests pass, ensuring correctness.

## Screenshots

- ![Simulator Running](screenshots/simulator.png)  
  Console output showing vehicle processing and light cycles.

- ![Graphics Window](screenshots/graphics.png)  
  SDL window displaying lanes, vehicles, and traffic lights (optional, requires SDL2 installation).

## Video Demo

[Watch the demo video](videos/demo.mp4)  
Demonstrates the simulation running with graphics and console output.

## Conclusion

The project successfully demonstrates queue-based traffic simulation with priority handling and IPC. Key achievements include O(1) queue operations, proportional serving, and cross-platform compatibility. Future enhancements could include multi-threading for concurrent generators, real-time graphics updates, and more complex priority schemes.

## Source Code

[GitHub Repository](https://github.com/sujan0629/dsa-queue-simulator)