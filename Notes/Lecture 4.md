# 1. Introduction to Scheduling

## 1.1 Definition
**Scheduling** in operating systems refers to the method by which an OS decides:
- Which task should be performed next,
- How long a task should perform before stopping or being interrupted,
- How to allocate limited resources (e.g., CPU cores, network cards, flash drives) among multiple requesting processes.

### Example Situations
- Multiple processes want to send messages via the same network card.
- Limited CPU cores are available, but there are more ready-to-run processes.

## 1.2 Goals of Scheduling
Scheduling decisions are based on **performance-related goals**, which can vary depending on the system's use case:
- **Throughput:** Maximize overall work done.
- **Response Time:** Minimize latency for interactive tasks.
- **Fairness:** Allocate resources equally among tasks.
- **Prioritization:** Allocate resources based on task importance.
- **Real-Time Guarantees:** Meet specific timing deadlines.

## 1.3 What is Scheduled?
Primarily: **Processes on cores**

However, scheduling also applies to:
- I/O Devices (e.g., disk scheduling)
- Network communication
- OS-level tasks


# 2. Process Queues and Scheduler Mechanisms

## 2.1 Process Descriptor and Ready Queue
Each process has an associated descriptor. Processes that are ready to run are enqueued in the **scheduler queue**, which can be designed to include:
- Only ready processes, or
- All processes with blocked ones at the end

Decisions include:
- When a blocked process is unblocked, should it be moved to the front of the queue?

## 2.2 Blocking and Ready States
- **Blocked processes**: Cannot run until a blocking I/O completes.
- **Ready processes**: Eligible to be scheduled.

## 2.3 Dispatcher and Context Switching
- **Dispatcher**: Mechanism that swaps process execution on cores.
- **Context Switch**: Operation of saving the current process’s state and loading the next.

## 2.4 Policy vs. Mechanism
- **Policy**: What to do (e.g., what process to run next).
- **Mechanism**: How to do it (e.g., how to load/save states, preempt processes).

| Component   | Role                                                    |
|------------|---------------------------------------------------------|
| Policy     | Defines which job should run and when                   |
| Mechanism  | Executes the low-level operations to run that decision |


# 3. Performance Metrics

## 3.1 Importance of Measurability
A performance goal must be:
1. **Quantitative** – represented with a number
2. **Measurable** – observable with tools or counters

## 3.2 Common Metrics
| Metric          | Description                                         |
|----------------|-----------------------------------------------------|
| Throughput      | Jobs completed per second                          |
| Response Time   | Delay between request and response                 |
| Turnaround Time | Time between job submission and completion         |
| Delay/Lateness  | Time past deadline or waiting time in queue        |

## 3.3 Ideal vs. Real Performance
### Throughput Curve
- Ideal: Linear increase up to capacity
- Real: Throughput peaks early and then degrades under overload

### Response Time Curve
- Ideal: Linear
- Real: Increases slowly until overload, then explodes to infinity

## 3.4 Graceful Degradation
A well-designed system should:
- Reduce performance gradually under overload
- Avoid zero throughput
- Avoid infinite queue times


# 4. Types of Scheduling

## 4.1 Preemptive vs. Non-Preemptive

| Type             | Description                                                           | Pros                                | Cons                                   |
|------------------|------------------------------------------------------------------------|-------------------------------------|----------------------------------------|
| Non-Preemptive   | Task runs until block/yield                                           | Simple, higher throughput           | Poor response, risk of starvation       |
| Preemptive       | OS can interrupt running process                                      | Fairness, real-time possible        | Complex, overhead, cache disruption    |

### Interrupts and Clock Interrupts
Used to:
- Preempt processes
- Perform time-slice-based preemptive scheduling

Common source: **Timer Interrupt** (e.g., 10ms intervals)

## 4.2 Cost of Context Switches
- Entering OS (hardware state change)
- Updating queues
- Changing memory maps (address spaces)
- Cache pollution


# 5. Non-Preemptive Scheduling Algorithms

## 5.1 First Come First Serve (FCFS)

- Jobs executed in arrival order
- Simple implementation using FIFO queue

### Example

| Process | Arrival | Duration |
|---------|---------|----------|
| P0      | 0       | 350ms    |
| P1      | 0       | 125ms    |
| P2      | 0       | 475ms    |
| P3      | 0       | 250ms    |
| P4      | 0       | 75ms     |

- Completion time: 1275 ms
- Average Wait Time: ~595 ms

### Pros and Cons

| Pros             | Cons                                |
|------------------|-------------------------------------|
| Simple           | Poor average response time          |
| High throughput  | Dependent on arrival/run time       |

## 5.2 Real-Time Scheduling

### 5.2.1 Hard Real-Time
- Must always meet deadline, failure = catastrophe
- Examples: Industrial control, nuclear systems

#### Characteristics
- Fully predictable execution times
- Static schedule
- Non-preemptive to ensure deterministic behavior
- Interrupts are avoided

### 5.2.2 Soft Real-Time
- Occasional deadline misses are allowed
- Focus: Minimizing missed deadlines or lateness

#### Techniques
- Earliest Deadline First (EDF)
- Job rejection
- Priority-based dropping of tasks

| Scenario           | Scheduler Action                              |
|--------------------|------------------------------------------------|
| Task late          | Drop, delay, or reschedule                   |
| Missed deadline    | Attempt to catch up if possible              |
| Less important     | Sacrificed to meet critical deadline         |


# 6. Preemptive Scheduling Algorithms

## 6.1 Round Robin

- Equal time quantum per process
- Clock interrupt triggers context switch

### Example (50ms time slice)

| Process | Duration |
|---------|----------|
| P0      | 350ms    |
| P1      | 125ms    |
| P2      | 475ms    |
| P3      | 250ms    |
| P4      | 75ms     |

- Time slices: Multiple rounds to complete
- Context Switches: 27
- Average Wait Time: ~100ms
- First job done at 475ms

| Metric             | FCFS    | RR       |
|--------------------|---------|----------|
| Context Switches   | 5       | 27       |
| Avg Wait Time      | 595ms   | 100ms    |
| First Job Done     | 350ms   | 475ms    |

### Time Slice Consideration

| Time Slice Length | Pros                          | Cons                            |
|-------------------|-------------------------------|---------------------------------|
| Short             | Better response time          | More overhead via switches      |
| Long              | Less overhead, higher throughput | Poor interactivity             |


## 6.2 Priority Scheduling

- Each process has a priority number
- Scheduler picks the highest-priority task

### Preemptive Behavior
- If a higher-priority task enters the system, it may preempt the current process.

### Starvation Problem
- Low-priority tasks may never execute

### Possible Solutions
- Aging: Gradually increase the priority of waiting processes
- Temporary priority boosting

## 6.3 Multi-Level Feedback Queue (MLFQ)

- Multiple queues, each with a different quantum
- New processes start in the highest-priority queue

### Behavior
- Interactive jobs stay in higher queues
- CPU-bound jobs migrate to lower queues
- Priorities decay over time or state

### Promotions/Demotions
| Trigger           | Action                    |
|------------------|---------------------------|
| Use full quantum | Demote to lower queue     |
| Frequent blocking| Remain in higher queue    |
| Starvation       | Periodic promotion        |

### Scheduling
- Round robin within each queue
- Higher priority queues are processed more frequently or exclusively

### Characteristics
- Adaptive: Tracks behavior of processes
- Balances fairness, throughput, responsiveness

### Diagram of Queue Transition (Example)

```
+--------------+--------------+--------------+
| High (10ms)  | Medium (50ms)| Low (100ms)  |
| - Responsive | - Mixed load | - CPU-bound  |
+--------------+--------------+--------------+
            ↓ demotion             ↑ promotion
```

| Type of Job      | Queue        | Reason                              |
|------------------|--------------|-------------------------------------|
| GUI handler      | High         | Needs immediate, frequent runs      |
| File I/O worker  | Medium       | Regular but delayed throughput      |
| Data mining tool | Low          | Long process, low interactivity     |


# Summary

This lecture addresses the crucial topic of **scheduling** in operating systems—deciding which process runs next and for how long, particularly under resource constraints. It examines **why scheduling is essential**, especially when processes must share limited hardware (CPU cores, flash drives, network cards). The discussion covers the **variety of performance goals** scheduling seeks to achieve, including **throughput**, **response time**, **fairness**, and **real-time guarantees**.

The material introduces the core mechanisms such as **process queues**, **dispatchers**, **context switching**, and the essential design principle of **separating policy from mechanism**. It further explores **non-preemptive scheduling algorithms** like First-Come First-Serve (FCFS) and special-purpose **real-time schedulers**, contrasting **hard** and **soft real-time systems**.

The lecture then delves into **preemptive scheduling**, distinguishing key methods such as **Round Robin**, **Priority Scheduling**, and the sophisticated **Multi-Level Feedback Queue (MLFQ)**. Each strategy is analyzed with practical examples to illustrate trade-offs involving **context switch overhead**, **fairness**, **adaptivity**, and **interactive responsiveness**. The lecture closes by emphasizing **graceful degradation** under high loads and reinforces that no one-size-fits-all algorithm exists—**the best choice depends on the system’s goals**.