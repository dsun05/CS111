# Lecture 4: Scheduling

## 1\. What Is Scheduling?

Scheduling is the process by which an operating system (OS) decides which task to perform next when there are multiple options available. This is particularly relevant for resources that can only serve one client at a time, like a CPU core, a network interface, or a disk drive. The fundamental questions scheduling answers are "Who gets to use the resource next?" and "For how long?".

### 1.1. Resources to Schedule

While this lecture primarily focuses on scheduling processes to run on CPU cores, the same principles apply to other system resources:

  * **CPU Cores**: Deciding which of the many ready processes gets to execute on an available core.
  * **Network Interface**: Determining the order in which to send multiple pending messages from different applications.
  * **Disk Drives**: Deciding the sequence for handling a set of requests to read or write data blocks for various files and processes.

### 1.2. The Process Queue

To manage scheduling, the OS maintains a **process queue**, which is a queue of all processes ready to run.

  * The queue is ordered according to the specific scheduling algorithm being used, ensuring the next process to run is at the head of the queue.
  * When a scheduling decision is made, the OS's **dispatcher** selects the process at the front of the queue to run.

#### Handling of Blocked Processes

Processes that are not ready to run (i.e., they are **blocked** waiting for a resource like I/O) must be handled. There are several design choices:

1.  **Separate Queues**: Maintain a queue that *only* contains processes that are ready to run. Blocked processes are kept elsewhere.
2.  **All-In-One Queue**: Keep all processes in a single queue, but ensure that blocked processes are moved to the end so they are not considered for scheduling.
3.  **Priority with Blocked State**: Keep important processes near the front of the queue even when they are blocked. The scheduler will ignore them until they become unblocked, at which point they can be run very quickly. This avoids the overhead of moving a process from a blocked queue or the back of the ready queue to the front upon unblocking.

A new process is added to the ready queue. A running process can be returned to the ready queue if it voluntarily yields or is preempted. If it makes a resource request, it may be moved to a blocked state managed by a resource manager until the resource is granted, at which point it returns to the ready queue.

-----

## 2\. Scheduling Goals

The choice of a scheduling algorithm is driven by the desired performance goals for the system. Different algorithms optimize for different metrics, and changing the algorithm can drastically alter system behavior.

Potential goals include:

  * **Maximize Throughput**: Complete the maximum amount of work for user processes in a given amount of time. This is a key goal for large-scale data processing systems like those used for training AI models.
  * **Minimize Average Waiting Time**: Reduce the average time a ready process spends waiting in the queue before it gets to run. This is crucial for interactive systems to ensure users don't experience long delays.
  * **Ensure Fairness**: Distribute CPU resources equitably among all processes. In a time-sharing system with many users, for example, each user should get an equal share of the CPU to ensure good response time and prevent favoring one student over another.
  * **Meet Priority Goals**: Give preferential treatment to more important processes over less important ones. This acknowledges that not all work is equally critical.
  * **Meet Real-Time Deadlines**: Ensure that time-sensitive tasks are completed by a specific deadline. This is essential for applications like video playback or industrial control systems, where missing a deadline leads to unacceptable quality or system failure.
  * **Meet Service Level Agreements (SLAs)**: In cloud computing, providers promise customers a certain amount of computing resources (CPU, memory, etc.). The scheduling algorithm must ensure these contractual agreements are met for all customers to avoid financial penalties.

-----

## 3\. Policy vs. Mechanism

In operating systems, it is desirable to separate policy from mechanism.

  * **Policy**: *What* should be done. It defines the goals. For example, the policy might be to prioritize interactive processes.
  * **Mechanism**: *How* to do it. This is the implementation that carries out the policy. For scheduling, the mechanism is the code that performs the dispatching—moving a process onto a core—and manages the process queue.

By separating them, the OS can support multiple policies with the same underlying mechanism, allowing it to switch policies as needed without rewriting code. For instance, a laptop might prioritize real-time guarantees when playing a video but switch to a different policy when the user is just reading a PDF.

-----

## 4\. Scheduling and Performance

Scheduling choices have a major impact on system performance. It's generally not possible to optimize all aspects of performance (e.g., throughput, response time, fairness) simultaneously. Performance also behaves very differently under a light load (not much work to do) versus a heavy load (more work to do than resources can handle).

### 4.1. Measuring Performance

To improve performance, goals must be quantitative and measurable. A **metric** is a combination of a characteristic to be measured (e.g., delay), a unit to quantify it (e.g., milliseconds), and a process for measuring it. You cannot optimize what you do not measure.

### 4.2. Throughput vs. Load

Ideally, as you offer more work (load) to a system, the amount of work it completes (throughput) increases linearly until it reaches the hardware's maximum capacity, at which point it plateaus.

In reality, the curve looks different. Throughput increases, but then tails off and can even decrease under very heavy load. This is because scheduling itself consumes resources (CPU cycles), which is known as **overhead**. The more frequently the scheduler runs, the more overhead it creates, leaving less time for actual user work and thus reducing throughput. To improve throughput, one must either reduce the overhead of each scheduling decision or reduce the number of decisions made.

### 4.3. Response Time vs. Load

Response time (or delay) is the time it takes to get a response to a request. Ideally, response time would increase linearly with load. However, in real systems, as the load approaches the system's capacity, the response time curve explodes towards infinity.

This explosion happens because real systems have finite resources, such as the amount of memory available for queues. When the rate of incoming requests (load) exceeds the rate at which they can be serviced, queues fill up. Once a queue is full, any new requests must be dropped. A dropped request is effectively a request with an infinite response time. This state is called **overload**.

### 4.4. Graceful Degradation
Graceful degradation is a principle in system design where a system continues to operate, albeit at a reduced level, when it becomes overloaded, rather than failing completely.

***

### Identifying Overload
A system is considered **overloaded** when it is no longer able to meet its predefined service goals. For example, if a system is designed to provide a response within 10 milliseconds, and it consistently fails to meet this target, it is in a state of overload.

***

### What to Do When Overloaded
When a system is overloaded, it should take specific actions to manage the situation gracefully:
* **Continue Service with Degraded Performance**: The system can keep providing service but with lower performance. For instance, the response time might increase from 10 ms to 20 ms, which is not ideal but better than no service at all.
* **Maintain Performance by Rejecting Work**: The system can choose to reject new incoming work to ensure it can still meet its performance goals for the tasks it has already accepted.
* **Resume Normal Service**: The ultimate goal is to return to normal operations as soon as the load decreases to a manageable level.

***

### What Not to Do When Overloaded
Certain behaviors should be avoided to prevent a complete system breakdown:
* **Do Not Let Throughput Drop to Zero**: The system should never stop doing useful work entirely. Even under heavy load, it should continue to make progress on user tasks. In poorly designed systems, all CPU time can be consumed by overhead tasks like dropping excess requests, leading to zero useful work being done.
* **Do Not Let Response Time Grow Without Limit**: For the work that is accepted, the system must ensure that the response time does not increase indefinitely. When queues become full, new requests are often dropped, which effectively results in an infinite response time for those specific requests. The system should prevent this from happening to all queued tasks.

-----

## 5\. Non-Preemptive vs. Preemptive Scheduling

This is a fundamental dichotomy in scheduling algorithms.

  * **Non-preemptive**: Once a process is given a resource, it keeps it until it voluntarily gives it up (by finishing, blocking, or yielding).
  * **Preemptive**: The operating system can forcibly take a resource away from a process to give it to another one.

| **Non-Preemptive Scheduling** | **Preemptive Scheduling** |
| --- | --- |
| **Pros:** Low scheduling overhead, high throughput, conceptually simple. | **Pros:** Good response time, excellent fairness, good for real-time and priority systems. |
| **Cons:** Poor response time, a buggy process with an infinite loop can freeze the system, poor fairness, difficult to implement priority or real-time guarantees. | **Cons:** More complex, requires mechanism to save and restore a process's state perfectly, can have lower throughput due to overhead from context switches. |

-----

## 6\. Non-Preemptive Scheduling Algorithms

### 6.1. First-Come, First-Served (FCFS)

FCFS is the simplest scheduling algorithm. Processes are added to the end of the queue as they arrive and are run in that order. Each process runs until it completes or yields.

  * **Properties**: It is simple and ensures that no process starves (waits forever), assuming no infinite loops. However, it can have highly variable and poor average waiting times. The "convoy effect" can occur, where short processes get stuck waiting behind a very long process.
  * **Use Cases**: FCFS works well when response time is not critical (e.g., old batch processing systems), when minimizing scheduling overhead is paramount (e.g., on very expensive supercomputers), or in simple embedded systems where tasks are brief and predictable.

#### FCFS Example

Consider five jobs arriving in order 0, 1, 2, 3, 4.
| Process | Duration (ms) | Start Time (ms) | End Time (ms) |
| :--- | :--- | :--- | :--- |
| 0 | 350 | 0 | 350 |
| 1 | 125 | 350 | 475 |
| 2 | 475 | 475 | 950 |
| 3 | 250 | 950 | 1200 |
| 4 | 75 | 1200 | 1275 |
| **Total** | **1275** | | |
| **Average Wait** | **595** | | |
The total run time is 1275 ms. The average wait time is high at 595 ms because the other four processes had to wait for the long-running Process 0 to finish.

### 6.2. Real-Time Schedulers

These algorithms schedule tasks based on deadlines. There are two types: hard and soft.

#### Hard Real-Time

In hard real-time systems, missing a deadline constitutes a system failure. These are used in safety-critical applications like controlling a nuclear power plant.

  * **Guaranteeing Deadlines**: To guarantee that no deadline is ever missed, systems are designed and analyzed exhaustively *before* they are run. The designers must know the exact execution time of every operation.
  * **Determinism**: The system must be perfectly predictable. Any source of non-determinism, such as interrupts, must be eliminated or controlled. This requires a **non-preemptive** scheduler that follows a pre-defined, static schedule of operations. All scheduling decisions are made at design time, not run time.

#### Soft Real-Time

In soft real-time systems, it is highly desirable to meet deadlines, but occasional misses are acceptable. Examples include streaming video or audio, where a missed deadline might result in a glitch but not a catastrophe.

  * **Goal**: The scheduler's goal is to minimize the number of missed deadlines or the total lateness.
  * **Handling Missed Deadlines**: When a deadline can't be met, the system might drop the task, allow the video to lag temporarily, or drop a future task to catch up.
  * **Algorithm**: A common algorithm is **Earliest Deadline First (EDF)**. Jobs are kept in a queue sorted by their deadlines, and the scheduler always runs the job with the nearest deadline. It is often implemented as a preemptive algorithm, so a new job with a more urgent deadline can interrupt the currently running one.

-----

## 7\. Preemptive Scheduling Algorithms

In preemptive scheduling, the OS can interrupt a running process.

### 7.1. Implications of Forcing Preemption

When the OS forces preemption, it interrupts a process that would otherwise continue running. This has several important implications. A process can be forcibly yielded at any time, for reasons such as:
* A more important, higher-priority process becomes ready to run, possibly after an I/O completion interrupt.
* The running process's own importance is lowered, for example, after it has been running for too long.

This forced interruption leads to several consequences:
* **Inconvenient State**: The OS does not know if it is a "convenient" time for the process to stop. The interrupted process might not be in a "clean" state from its own perspective. While the OS always stops a process after a complete instruction, ensuring the state is valid, the process could be in the middle of a complex, multi-instruction operation.
* **Enforced Fairness**: The ability to preempt is what enables enforced "fair share" scheduling.
* **Gratuitous Context Switches**: Preemption introduces context switches that are not required by the process's own logic (the process did not yield or block). These are called "gratuitous" context switches and are considered overhead, which can negatively impact throughput.
* **Resource Sharing Problems**: Forcing a process to stop can create potential resource sharing and synchronization problems. If a process is interrupted while it holds a lock on a shared resource, complex issues can arise.

***

### 7.2. Implementing Preemption

To preempt a process, the OS needs a way to regain control of the CPU from that process. This is achieved through **interrupts**.

### General Interrupt Handling

Any time a hardware interrupt occurs—such as a network card signaling a new message has arrived or a disk controller indicating an I/O operation is complete—the currently running process is halted, and the CPU switches to executing the OS's interrupt handler code. After handling the specific needs of the interrupt, the OS has an opportunity to run the scheduler before returning control to a user process.

At this point, the scheduler can evaluate if the system state has changed in a way that warrants preemption. For instance:
* Did the interrupt unblock a process with a higher priority than the one that was just running?
* Should the priority of the currently running process be lowered for some reason?

If the scheduler determines a higher-priority process is now ready, it will perform a context switch to run that new process. Otherwise, it will return control to the process that was originally interrupted.

### Clock Interrupts

While general interrupts provide opportunities for preemption, the key technology that guarantees it is the **clock interrupt**. Modern CPUs contain a built-in programmable timer that can be configured by the OS to generate an interrupt after a specified interval, often called a **time slice** or quantum.

When the timer expires, it triggers an interrupt that forces the CPU to stop the current process and execute the OS's clock interrupt handler. This handler's primary purpose is to invoke the scheduler, which will then typically move the just-interrupted process to the ready queue and select the next process to run.

This mechanism is crucial for two main reasons:
1.  **Enforces Time-Based Policies**: It allows the OS to implement fair-share algorithms like Round Robin by ensuring every process gets a turn.
2.  **Prevents System Freezes**: It provides a failsafe against buggy processes. A process stuck in an infinite loop cannot monopolize a core indefinitely, because the clock interrupt will eventually fire, allowing the OS to regain control and schedule other processes.

***

### 7. 3 Choosing a Time Slice

The length of the time slice is a critical parameter.

* **Long Time Slices**: Fewer context switches, leading to lower overhead and higher throughput. However, response time suffers, and the system behaves more like FCFS.
* **Short Time Slices**: Better response time, but more context switches, which increases overhead and reduces throughput.
    The balance often depends on the cost of a **context switch**. This cost includes not just the OS code for saving/restoring state, but also indirect costs like losing the contents of the CPU caches (instruction and data caches), which can dramatically slow down the newly scheduled process.
  
***

### 7.4 The Costs of a Context Switch

A context switch is an expensive operation that consumes system resources without performing any user work, making it pure overhead. The costs include:

* **OS Entry and Scheduler Execution**: A switch begins with an interrupt or system call, which transfers control to the OS. The OS must save the running process's registers and then execute the scheduler code to decide which process runs next. This sequence of operations consumes CPU time.

* **Saving and Restoring Context**: The OS must save the full context of the process being taken off the CPU and then load the full context of the new process. This includes managing their respective stacks and process descriptors.

* **Address Space Switching**: Each process has its own virtual address space. During a context switch, the OS must reconfigure the memory management unit (MMU) to map the new process's address space, an operation which has associated costs.

* **Cache Invalidation**: This is arguably the most significant performance cost of a context switch on modern computers. CPUs rely heavily on fast on-chip caches to avoid slow access to main memory. When a process runs, it "warms up" the cache with its own instructions and data. After a context switch, this cached data belongs to the old process and is useless to the new one. The new process will therefore suffer a high number of cache misses, forcing it to retrieve data from much slower main memory and significantly slowing its initial execution until its own data populates the cache.

***

### 7.5. Round Robin (RR)

The goal of Round Robin is to provide fair and equal shares of the CPU to all processes.

* **Time Slice (Quantum)**: Each process is given a small unit of time to run, called a time slice or quantum.
* **How it Works**: The scheduler runs the process at the head of the queue. If the process blocks or finishes before its time slice expires, the scheduler runs the next process. If the time slice expires, a clock interrupt occurs, and the scheduler moves the current process to the end of the queue and runs the next one.
* **Properties**: RR provides good response time for interactive processes because every process gets a chance to run quickly. The main downside is higher overhead due to frequent context switches.

#### Round Robin Example

Using the same five jobs as the FCFS example, with a 50 ms time slice.

| Process | Length (ms) | Finish Time (ms) | Switches |
| :--- | :--- | :--- | :--- |
| 0 | 350 | 1100 | 7 |
| 1 | 125 | 550 | 3 |
| 2 | 475 | 1275 | 10 |
| 3 | 250 | 900 | 5 |
| 4 | 75 | 475 | 2 |
| **Totals** | **1275** | **1275** | **27** |

#### Comparison: FCFS vs. Round Robin

| Metric | FCFS | Round Robin | Analysis |
| :--- | :--- | :--- | :--- |
| **Context Switches** | 5 | 27 | RR has much higher overhead. |
| **First Job Completed** | 350 ms (P0) | 475 ms (P4) | RR can take longer to finish the first job. |
| **Average Waiting Time** | 595 ms | ~100 ms | RR is far more responsive, with much lower average wait to first compute. |

***

### 7.6. Priority Scheduling

This algorithm acknowledges that some processes are more important than others.

* **How it Works**: Each process is assigned a priority number. The scheduler always runs the ready process with the highest priority.
* **Preemptive Priority**: In a preemptive system, if a new process arrives that has a higher priority than the currently running process, the scheduler will interrupt the running process and start the new, higher-priority one.
* **Starvation**: A major problem with priority scheduling is **starvation**. A low-priority process might never get to run if there is a continuous stream of higher-priority processes arriving. To solve this, systems often use **priority aging**, where the priority of a process that has been waiting for a long time is temporarily increased, and the priority of a process that has been running for a long time is temporarily lowered.

***

### 7.7. Multi-Level Feedback Queue (MLFQ)

The **Multi-Level Feedback Queue (MLFQ)** scheduler is an adaptive algorithm designed to achieve two conflicting goals simultaneously: provide quick response times for interactive jobs and high throughput for long-running, CPU-bound jobs. It achieves this without requiring prior knowledge of a process's behavior by observing how processes act over time and adjusting their priority accordingly.

### The Queues

MLFQ uses a hierarchy of multiple ready queues, each with a different priority level.

  * **High-priority queues** are given short time slices (quanta).
  * **Low-priority queues** have longer time slices.

A process in a high-priority queue will always be chosen to run over a process in a lower-priority queue. Processes within the same queue are scheduled against each other using a simple Round Robin algorithm.

-----

### The Rules of MLFQ

MLFQ operates based on a set of rules that govern how processes move between queues.

| Rule | Description |
| :--- | :--- |
| **1. Priority** | A process with higher priority runs before a lower-priority process. Processes in the same queue are scheduled using Round Robin. |
| **2. Entry** | All new processes are placed in the highest-priority queue to give them the best initial response time. |
| **3. Demotion** | If a process uses its *entire* time slice allocation at one level, the OS assumes it is a long-running, CPU-bound job and moves it down to the next lower-priority queue. |
| **4. I/O Behavior** | If a process blocks for I/O *before* its time slice expires, the OS assumes it is an interactive job. It remains in the same queue when it becomes ready again, preserving its priority. |
| **5. Priority Boost** | To prevent starvation of processes that have been demoted to low-priority queues, the scheduler periodically moves all processes back to the highest-priority queue. |

-----

### Visualizing MLFQ

The movement of processes in an MLFQ system can be visualized as follows:

```
                                     +--------------------------------+
                                     |         New Processes          |
                                     +--------------------------------+
                                                 |
                                                 V
+--------------------------+         +--------------------------------+
| Priority Boost           |<--------|  Queue 1 (High Priority)       |
| (All jobs move to top)   |         |  Time Slice = 10ms             |
+--------------------------+         +--------------------------------+
                                      |          |                   ^
(Uses full 10ms)                      |          | (Blocks for I/O)  |
 V                                    |          +-------------------+
+--------------------------------+    |
|  Queue 2 (Medium Priority)     |<---+
|  Time Slice = 50ms             |
+--------------------------------+
  |
  | (Uses full 50ms)
  V
+--------------------------------+
|  Queue 3 (Low Priority)        |
|  Time Slice = 200ms            |
+--------------------------------+
```
