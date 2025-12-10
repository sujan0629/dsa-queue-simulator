# DSA Queue Simulator - Traffic Light Management System

## Data Structures Used

| Data Structure | Implementation | Purpose |
|----------------|-----------------|---------|
| Queue | Linked List (Node with Vehicle, front/rear pointers, size) | Store vehicles in each lane FIFO order |
| Vehicle | Struct with int id | Represent individual vehicles |
| LightState | Enum (RED, GREEN) | Track traffic light state |

## List of Functions

- `createQueue()`: Initialize queue
- `enqueue(Queue*, Vehicle)`: Add vehicle to queue
- `dequeue(Queue*)`: Remove and return vehicle from queue
- `isEmpty(Queue*)`: Check if queue empty
- `getSize(Queue*)`: Get queue size
- `freeQueue(Queue*)`: Deallocate queue
- `estimate_pass_time(int)`: Calculate time for vehicles to pass