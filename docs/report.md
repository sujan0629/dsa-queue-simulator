# DSA Queue Simulator - Traffic Light Management System

## Data Structures Used

| Data Structure | Implementation | Purpose |
|----------------|-----------------|---------|
| Queue | Linked List (Node with Vehicle, front/rear pointers, size) | Store vehicles in each lane FIFO order |
| Vehicle | Struct with int id | Represent individual vehicles |
| LightState | Enum (RED, GREEN) | Track traffic light state |

## Algorithm Used

The simulation uses a priority queue approach for traffic management:

1. Vehicles are generated and enqueued in lane-specific queues.
2. Traffic light cycles between RED and GREEN states.
3. During GREEN, check for priority lane (AL2) if >10 vehicles.
4. If priority, serve until <5 vehicles.
5. Else, serve proportionally: |V| = (total_vehicles / num_lanes), round-robin.
6. Poll every second, load new vehicles, truncate files.

## Time Complexity

- Enqueue/Dequeue: O(1) per operation
- Priority check: O(1) (only one lane)
- Normal serving: O(n) where n=4 lanes
- Overall simulation: O(t) where t is time steps, dominated by polling

Space: O(total vehicles) for queues.