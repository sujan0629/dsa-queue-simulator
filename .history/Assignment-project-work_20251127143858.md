# Data Structure and Algorithms (COMP202)

## Assignment #1
### Implementing Queue for solving the traffic light problem

**Course:** Data Structure and Algorithms (COMP202)  
**Date:** Nov 2025

---

## 1. Problem Description

A traffic junction connects two major roads, forming a central point where vehicles must choose one of three alternative paths to continue (visual representation is shown in figure 1). The traffic management system must handle the following scenarios:

- **Normal Condition**: Vehicles at the junction are served equally from each lane. The system should ensure that vehicles are dispatched fairly.

- **High-Priority Condition**: If one of the roads (referred to as the priority road) accumulates more than 10 waiting vehicles, that road should be served first until the count drops below 5. Afterward, normal conditions resume.

---

## 2. Objective

Following are the objectives of the assignment:

- Use linear data structure to solve real world problem
- Visualize or Simulate the queue management system (in terms of traffic management system)

---

## 3. Details

### 3.1 Road and Lanes

There are four major roads ({A, B, C, D}) with three lanes each.

- The first lane (e.g., **AL1**) of each road are incoming lanes.
- Other two are the outgoing lanes.
- The third lane is a free lane and only allowed to turn left.
- The vehicle of second lane of each road need to follow the various light conditions.
- The lane **AL2** is a **priority lane**.

The details of the road is shown in figure 1.

**Figure 1:** Visual representation of the junction

### 3.2 What is normal lane?

All lanes are normal lanes until explicitly declared as priority. This lane will be served on normal condition based on the average number of vehicles that are waiting on normal lanes.

**Formula:**

```
Vehicle Served at once (|V|) = (1/n) × Σ(i=0 to n) |Li|
```

Where:
- **n**: Total number of lanes (in this case it is 3 ⇒ BL2, CL3, DL4)
- **|Li|**: Total number of vehicles waiting on i-th lane
- **|V|**: Total number of vehicles served

It is hard to count the number of vehicles passes the junction. To solve this problem we have to estimate the time required for passing m number of vehicles.

**Total time of green light (state 2) = |V| × t**

Where:
- **t**: Estimated time required to pass one vehicle

### 3.3 What is priority lane?

This lane is a special lane where the waiting time should be low. When there are more than 5 vehicles are waiting this lane should be served immediately after running lighting condition ends. 

**Special Case:** For the vehicle less than 5, this lane also turns into the normal lane.

### 3.4 Traffic Light

For the simplicity, there will be total of four traffic lights each of them will instruct the vehicle of opposite lane.

**Red Light**: State 1 ⇒ Stop  
**Green Light**: State 2 ⇒ Go straight or turn

The light on each road will be of pair. Solid light or arrow light indicating stop or go.

**Figure 2:** Light Conditions

When any road's light turned into State 2, all others should be in state 1 to avoid **deadlock**.

---

## 4. Programming Requirements

You need to extend the given source code to make a simulator. The data storage and operation for the vehicle to process, and lane to process are need to be encoded in the queue structure.

### 4.1 Queues

Basically, you may need two types of queue:

1. **Vehicle Queue**: There should be 4 different vehicle queues maintaining the list of the vehicles to be processed

2. **Lane/Light Queue**: This should be a priority queue. Initially all the elements will have same priority. Once **AL2**'s number of elements is more than 10, this element should be updated with highest priority.

### 4.2 Programs

Consider following during implementation:

- **simulator.c**: The main program will have graphics related code and your main logic to process the queue and visualize them

- **traffic_generator.c**: This program will be responsible to generate the vehicle on each lane.

### 4.3 Communication

Both programs need to communicate. This can be done via following three approaches (easiest to hardest):

- **Sharing of the files**: Generator will write on the respective lane files (say: {lanea, laneb, lanec, laned}.txt file) and simulator will continuously poll for those files and update the respective lane queue.

- **Inter Process Communication**: Use the interprocess communication to generate and pass the vehicle data to main program

- **Socket**: Use the network socket program for communication

### 4.4 Libraries need to explore

- **Simple DirectMedia Layer 2.0**: Documentation: https://wiki.libsdl.org/SDL2/FrontPage

---

## 5. Submission of Assignment

You need to submit the link to the git repository. This should be public.

### 5.1 Report

This is the documentation of your work. Include following sections:

- **Title**: Assignment number, your name and roll number
- **Summary of work**: Describe the summary of the work you have done
- **Data structure**: Table showing data structure used, how it is implemented and purpose
- **List of the functions**: List of the functions you have implemented that uses the data structure
- **Algorithm used**: Algorithm used for processing the traffic
- **Time complexity**: Time complexity of your algorithm with detailed explanation
- **Link to the source code**

### 5.2 Submission

You need to submit the source code in **GitHub**. The submission details:

- Make a public repository (**dsa-queue-simulator**)
- Commit your code gradually on this repository - 20-30 commits are expected
- Maintain the **README** file
  - Write a process to run your program
  - Include gif/video showing the final output, running both programs
  - Include the reference you used to implement the algorithms or any source code in readme file
  - Include other details whatever necessary and required