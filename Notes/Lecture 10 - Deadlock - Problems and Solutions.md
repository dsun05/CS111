# Lecture 10: Deadlock - Problems and Solutions

## 1.0 The Deadlock Problem

### 1.1 Definition of Deadlock

A **deadlock** is a state in which two or more concurrent entities (such as processes or threads) are blocked indefinitely, each waiting for a resource that is held by another entity in the same set. Because all entities are waiting, none can proceed, and the system effectively halts.

For a deadlock to occur, two or more entities must each have locked a resource and need the other's locked resource to continue. Since neither will unlock its resource until it acquires the other's, no progress can ever be made.

***
### 1.2 The Dining Philosophers Problem

The Dining Philosophers Problem is a classic, artificial problem in computer science used to illustrate deadlock and synchronization issues.

*   **Setup:** Five philosophers are seated at a circular table. In front of each philosopher is a plate of pasta, and between each pair of adjacent philosophers is a single fork. There are five philosophers and five forks in total.

*   **Rules:**
    1.  A philosopher alternates between thinking and eating.
    2.  To eat, a philosopher needs to use two forks: the one on their left and the one on their right.
    3.  A philosopher can only pick up one fork at a time.
    4.  The philosophers do not communicate or negotiate with each other.

*   **Deadlock Scenario:** A deadlock can occur if every philosopher decides to eat at the same time and each one picks up the fork to their right. At this point, every philosopher is holding one fork and is waiting for the fork on their left. Since the left fork is held by their neighbor, and no one will release a fork until they have eaten, all philosophers will wait forever.

*   **Purpose:** The problem is not meant to be realistic but serves as a model to understand how deadlock arises and to explore solutions by modifying the rules or the behavior of the philosophers (e.g., by changing how they acquire forks).

***
### 1.3 Importance and Challenges of Deadlocks

Deadlocks are a major peril in real-world parallel and distributed systems.

*   **Consequences:** When a deadlock occurs in a real system, it can be catastrophic, causing critical processes to hang and the system to become unresponsive until the deadlock is broken, which often requires a full system restart.
*   **Difficulty in Debugging:** Deadlocks can be intermittent and depend on specific timing, making them hard to reproduce and diagnose. In complex systems with numerous locks, identifying the specific combination causing the deadlock is challenging. They are much easier to prevent at design time.

*   **Deadlocks May Not Be Obvious:** Deadlocks are difficult to detect because they often arise from dynamic system interactions rather than static code defects.
    *   **Process resource needs are ever-changing.** Locking patterns are not fixed and can be unpredictable, as they depend on:
        *   **Data being operated on:** Different data can trigger different execution paths, leading to varied resource locking sequences.
        *   **The current stage of computation:** A process in an initialization phase has different locking needs than one in an error-handling phase.
        *   **The occurrence of errors:** Exceptional error paths may have unique locking patterns that conflict with normal operations.
    *   **Modern software depends on many services.** These services are often complex and operate without knowledge of one another.
        *   Each service may require numerous internal resources and implement its own locking.
        *   When these independent services are combined, their locking strategies can interact in unforeseen ways, creating deadlocks that were impossible for the individual component developers to predict.
    *   **Services encapsulate complexity.** Good software design principles like abstraction hide the very details needed to identify potential deadlocks.
        *   It is often unknown what specific resources a service call will require.
        *   Crucially, the *order* in which a service acquires its internal locks (its serialization) is a hidden implementation detail, making it impossible to enforce a system-wide locking order to prevent circular waits.

***
### 1.4 Resource Types and Deadlock

Deadlocks can involve different types of resources, which are handled with different strategies.

*   **Commodity Resources:** These are pools of interchangeable units of a resource, such as memory or disk space. A client needs a certain *amount* of the resource, not a specific unit.
    *   Deadlocks typically arise from **over-commitment**, where the total amount of resources promised (reserved) exceeds what is available, and multiple processes block waiting for their allocation.
    *   The primary strategy for handling these is **deadlock avoidance**, typically implemented by a central resource manager.

*   **General Resources:** These are specific, unique instances of a resource, such as a particular file, a data structure, or a semaphore. A client needs a lock on that *specific instance*.
    *   Deadlocks result from specific **dependency relationships** between processes requesting these unique resources.
    *   The primary strategy is **deadlock prevention**, which is implemented at design time by structuring the code to avoid the conditions that lead to deadlock.

***
### 1.5 The Four Necessary Conditions for Deadlock

For a deadlock to occur, all four of the following conditions must be met simultaneously. If any one of these conditions is not met, a deadlock is not possible.

1.  **Mutual Exclusion:** At least one resource must be held in a non-shareable mode. Only one process at a time can use the resource. If another process requests that resource, the requesting process must be delayed until the resource has been released.
2.  **Incremental Allocation (Hold and Wait):** This condition is met when a process, already allocated one or more resources, requests an additional resource and blocks because it is currently in use. The process continues to "hold" its existing resources, preventing other processes from using them while it waits. The alternative is **pre-allocation**, which requires a process to acquire all necessary resources simultaneously before it begins its work, thus preventing the "hold and wait" scenario.
3.  **No Preemption:** Resources cannot be preempted; that is, a resource can only be released voluntarily by the process holding it, after that process has completed its task.
4.  **Circular Wait:** There must exist a set of waiting processes {P₀, P₁, ..., Pₙ} such that P₀ is waiting for a resource held by P₁, P₁ is waiting for a resource held by P₂, ..., and Pₙ is waiting for a resource that is held by P₀.

***
### 1.6 Wait-For Graphs

A **wait-for graph** is a directed graph used to detect deadlocks.

*   **Nodes:** The nodes of the graph represent the active processes.
*   **Edges:** A directed edge from process Pᵢ to process Pⱼ exists if Pᵢ is blocked and waiting for a resource currently held by Pⱼ.

**A deadlock exists in the system if and only if there is a cycle in the wait-for graph.**

*   **Example:**
    1.  Thread 1 acquires a lock for Critical Section A.
    2.  Thread 2 acquires a lock for Critical Section B.
    3.  Thread 1 requests the lock for Critical Section B (held by Thread 2) and blocks. The graph has an edge from Thread 1 → Thread 2.
    4.  Thread 2 requests the lock for Critical Section A (held by Thread 1) and blocks. The graph adds an edge from Thread 2 → Thread 1.
    5.  The graph now contains a cycle (Thread 1 → Thread 2 → Thread 1), indicating a deadlock.

---
## 2.0 Handling Deadlocks

There are several strategies for dealing with deadlocks, which can be broadly categorized as avoidance, prevention, and detection/recovery.

***
### 2.1 Deadlock Avoidance

Deadlock avoidance is a strategy that ensures the system will never enter a deadlock state. This is typically used for **commodity resources** where units are interchangeable (e.g., memory, disk space). The system uses algorithms to analyze resource requests and makes decisions that guarantee safety.

***
#### 2.1.1 Advance Reservations and Safe Allocation

The core of deadlock avoidance is requiring processes to declare their potential resource needs in advance through **reservations**.

*   **Rationale:** Processes often cannot perfectly predict their resource needs and, to be safe, tend to reserve for a worst-case scenario that is rarely encountered. Most of the time, they use far less than their maximum reservation. This behavior is what makes strategies like overbooking feasible.

*   **Safe Allocation:** A resource manager only grants a reservation if it can guarantee the system will remain in a **safe state**. A state is considered safe if the system can find at least one sequence of allocations that allows every process to eventually run to completion. This does not mean there will be no waiting, but it guarantees there will be no deadlock.

*   **Real-World Examples:** This mechanism is common in real systems:
    *   **Disk Quotas:** A user is reserved a certain amount of disk space.
    *   **Quality of Service (QoS) Contracts:** In cloud computing, a client contracts for a reserved amount of CPU, network bandwidth, or I/O operations per second.
    *   **Airline Seating:** Airlines famously overbook flights, reserving more seats than are physically available based on the statistical probability that some passengers will not show up.

***
#### 2.1.2 The Dilemma: Overbooking vs. Under-utilization

Resource managers face a fundamental trade-off:

*   **Under-utilization:** If the manager only grants reservations up to its physical capacity, many resources will sit idle, as most processes will not be using their full worst-case reservation.
*   **Overbooking:** The manager can grant reservations that, in total, exceed its physical capacity. This leads to higher resource utilization but introduces the risk that the system may not be able to fulfill a promised reservation immediately if too many processes request their maximum allocation at once.

***
#### 2.1.3 Handling Reservation Failures

A system built on reservations changes *when* failures occur.

*   **Predictable Failures:** Once a reservation is granted, the system must guarantee it can be honored. This means allocation failures happen *only at reservation time*, not when the process later requests the resource. This makes system behavior more predictable and easier to handle.

*   **Application Design:** Applications must be designed to handle reservation failures gracefully.
    *   The application should be able to continue running, perhaps by refusing to perform a new request that needs the unavailable resource.
    *   It must have a way to report the failure to the user or client (e.g., via error messages or return codes).
    *   A robust design will reserve all absolutely critical resources at start-up to ensure it can perform its core functions.

*   **Why Rejecting Early is Better:** It may seem bad to reject a request, but it is far better than failing later. If an application fails mid-operation, it may have already allocated other resources and left data in a partially modified, inconsistent state. Unwinding this half-completed work can be extremely complex or even impossible. Early rejection gives the application a chance to adapt, perhaps by trying a different approach that doesn't need the unavailable resource.

***
### 2.2 Deadlock Prevention

Deadlock prevention involves structuring the system to ensure that at least one of the four necessary conditions for deadlock cannot hold for any lock. Unlike deadlock avoidance, which analyzes the global state, prevention applies design rules to make deadlocks structurally impossible. This is typically used for **general resources**.

***
#### 2.2.1 Attacking Mutual Exclusion

This is often the most difficult condition to eliminate, as many resources are inherently non-shareable. However, strategies include:
*   Making resources shareable wherever possible (e.g., using read-only access).
*   Using atomic hardware instructions to perform updates, which may remove the need for a long-held lock.
*   Giving each process its own private copy of a resource, though this is often impractical.

***
#### 2.2.2 Attacking Incremental Allocation (Hold and Wait)

The core idea is to prevent a process from holding resources while being blocked waiting for another.
1.  **Allocate All Resources at Once:** A process must request and be allocated all its resources in a single, "all-or-nothing" operation. If the complete set of resources cannot be granted, the process gets none and blocks without holding any.
2.  **Use Non-Blocking Requests:** Design resource requests to fail immediately rather than blocking if the resource is not available. The process can then release its held locks and retry later, preventing the "wait" part of the condition.
3.  **Release Locks Before Blocking:** A process must release all held locks *before* executing any operation that might cause it to block (including I/O or other synchronization events). After the block is over, it must re-acquire the locks. While this prevents deadlock, it can be inefficient and may introduce other problems like starvation.

***
#### 2.2.3 Attacking No Preemption (Allowing Resource Confiscation)

This strategy breaks deadlocks by forcibly taking resources away from a process.
*   **Mechanism:** This is often implemented with **leases**—locks that have a built-in time-out. The system can also have explicit "lock breaking" capabilities.
*   **Challenges and Requirements:**
    *   **Enforcement is Critical:** The system must be able to *enforce* revocation. If a process accesses a resource via a system service, that service can stop honoring requests. If the process has direct memory access, the only way to enforce revocation may be to kill the process.
    *   **Resource Integrity Risk:** A process whose lock is confiscated may have been in the middle of a multi-step update, leaving the resource in an inconsistent or damaged state. This requires the system to have mechanisms to audit and repair the resource or roll it back to a known-good state.
    *   **Resource Design:** Ultimately, a resource must be *designed* with the possibility of revocation in mind.


***
#### 2.2.4 Attacking Circular Wait Through Total Resource Ordering

This is one of the most practical prevention techniques. A global order is imposed on all lockable resources (e.g., by assigning each a unique number), and processes must request locks in ascending order. A process holding lock Lᵢ can request lock Lⱼ only if j > i. This structure makes a circular wait impossible.

However, this simple rule creates a significant challenge when a process holding a high-order lock needs a low-order lock. To comply, it must perform a **lock dance**.

##### Example: The Lock Dance in Linked List Management

This problem is common in systems that manage a pool of resources, such as a linked list of data buffers. Such a system has two levels of locking:

*   **The List Structure Lock:** A single, low-order lock (e.g., Lock #10) that protects the list's overall structure (the `next`/`previous` pointers). It must be held to add or remove nodes safely.
*   **Individual Buffer Locks:** Each buffer has its own high-order lock (e.g., #100, #101, etc.) that protects the data *within* that buffer.

**The Conflict:** A thread holds a lock on a specific buffer (#101) and now needs to **delete the buffer from the list**. This operation represents the finalization of a resource, such as when its data has been written to disk and the buffer can be removed from a cache. Executing this deletion requires modifying the list structure (protected by lock #10), but the ordering rule prohibits requesting lock #10 while holding #101.

**The Solution (The Lock Dance):**
To proceed, the thread must execute the following sequence:
1.  Release the high-order **buffer lock** (#101).
2.  Acquire the low-order **list head lock** (#10).
3.  Re-traverse the list to find the buffer again and re-acquire its **buffer lock** (#101).
4.  Perform the deletion now that both locks are held in the correct order.
5.  Release both locks.

**The Consequence: Trading Deadlocks for Race Conditions**
The lock dance solves the deadlock problem but introduces a new, serious risk: a **race condition**.
*   **The Window of Vulnerability:** Between releasing the buffer lock (Step 1) and re-acquiring it (Step 3), the buffer's data is unprotected.
*   **The Risk:** Another thread could acquire the lock in this interval and modify the buffer's data. When the original thread re-acquires the lock, it may be operating on stale or corrupted data.

Therefore, deadlock prevention through resource ordering is not a "free" solution; it forces developers to write more complex code and guard against different kinds of concurrency bugs.

***
### 2.3 The "Ostrich Algorithm": Ignoring the Problem

This is a common, pragmatic approach where the developer assumes deadlocks are sufficiently rare and that the performance and complexity costs of prevention or avoidance are too high. The system is built with no specific mechanism to handle deadlocks. If a deadlock occurs, the system will hang and likely require a manual restart.

***
### 2.4 Deadlock Detection and Recovery

This strategy is fundamentally different from prevention and avoidance. Instead of stopping deadlocks from ever happening, this approach is reactive: it allows deadlocks to occur, provides a mechanism to detect them after the fact, and then initiates a recovery process to break them. This is often the implicit fallback for systems that use the "Ostrich Algorithm" (ignoring the problem) until a hang actually occurs.

***
#### 2.4.1 The Detection Mechanism

The core of deadlock detection is identifying a circular wait condition in the system's resource allocation state.

*   **The Wait-For Graph:** Detection is typically implemented by maintaining a **wait-for graph**. A background process can periodically check this graph for cycles, or a check can be triggered each time a process blocks on a resource request. If a cycle is found, a deadlock is confirmed.

*   **Challenges of Implementation:**
    1.  **Resource Visibility:** This is the greatest challenge. To be effective, the detector must be aware of *all* lockable resources and all lock requests. In a modern OS, this is nearly impossible. Many locks are implemented at the application or library level (e.g., in a multi-threaded program's user space) and are completely invisible to the OS kernel. The OS cannot build an accurate wait-for graph for resources it doesn't manage.
    2.  **Performance Overhead:** Maintaining and repeatedly searching a potentially large and rapidly changing wait-for graph introduces significant performance overhead.
    3.  **The Prevention Paradox:** If a system is sophisticated enough to update the graph and detect a cycle the moment a lock request would create one, why not just *reject* that lock request and prevent the deadlock? This makes detection as a purely post-mortem tool less appealing than using the same mechanism for prevention.

***
#### 2.4.2 The Recovery Mechanism

Once a deadlock is detected, the system must break the cycle. This is typically disruptive.

*   **Process Termination:** The simplest and most brute-force method. The system aborts one or more processes involved in the deadlock cycle. This breaks the wait chain but results in lost work. This raises the **victim selection problem**:
    *   Which process should be killed? The one with the lowest priority? The one that has consumed the least CPU time? The one that is holding the fewest resources? There is no single easy answer.

*   **Resource Preemption:** A more graceful but far more complex approach. The system forcibly takes a resource away from one of the deadlocked processes (the victim) and gives it to another. This requires a robust **rollback mechanism**. The victim process was interrupted mid-operation and its state must be carefully rewound to a point *before* it acquired the preempted resource to ensure data consistency.

***
#### 2.4.3 When Detection and Recovery Is a Good Approach: Database Systems

While difficult for a general-purpose OS, deadlock detection and recovery is the standard approach in database management systems. This is because databases overcome the primary challenges:

1.  **A Self-Contained Universe:** A database system has complete visibility and control over its own resources. It manages every lock on every table, page, and row. It knows about every waiting transaction and can build a perfect wait-for graph for its own domain.
2.  **Built-in Recovery via Transactions:** Databases are built around the concept of **transactions**, which are atomic and can be rolled back. **Transaction rollback** is a core, well-tested feature. When a deadlock is detected between two transactions, the database's recovery is simple and natural:
    *   It selects one transaction as the victim.
    *   It rolls back all of that transaction's changes, releasing its locks.
    *   It notifies the client that the transaction failed and should be retried.

This makes deadlock detection and recovery a highly effective and practical strategy within the controlled environment of a database.

---
## 3.0 General Synchronization Bugs and Health Monitoring

Not all system hangs are deadlocks. Other synchronization problems exist, such as livelock, priority inversion, and missed wakeups. Deadlock-specific solutions will not solve these.

***
### 3.1 Health Monitoring

Health monitoring is a more general and practical approach that detects a wide range of problems, not just deadlocks. It involves building mechanisms to observe whether an application is running correctly.

***
#### 3.1.1 Monitoring Techniques

Effective health monitoring employs a layered approach, using different techniques to check a system's status.

*   **1. Look for Obvious Failures:** This is the most basic check. The monitoring system looks for clear signals of catastrophic failure, such as unexpected process exits or the generation of a core dump file, which indicates a crash.

*   **2. Passive Observation to Detect Hangs:** Without directly interacting with the process, the monitor observes its resource consumption from the operating system's perspective.
    *   **CPU Time:** Is the process consuming 100% of a CPU core, suggesting an infinite loop? Or is it consuming 0% CPU time, suggesting it is blocked?
    *   **I/O Activity:** Is the process performing network and/or disk I/O as expected for its workload? A complete lack of I/O from a process that should be active can indicate a hang.

*   **3. External Health Monitoring (Black-Box Probing):** The monitor actively probes the target application or service from the outside to test its responsiveness and correctness.
    *   **"Pings":** A simple "are you alive?" message sent to the service. A response confirms that the process is at least running and able to process basic network input.
    *   **Null Requests:** A well-formed, valid request that requires minimal work (e.g., a "get status" command). A correct response confirms the service's basic logic is operational.
    *   **Standard Test Requests:** A request with a known input that should produce a known, verifiable output. This tests not just responsiveness but functional correctness.

*   **4. Internal Instrumentation (White-Box Auditing):** The application is designed to monitor itself. This provides the most detailed information.
    *   **White-Box Audits:** The process periodically runs internal routines to check the consistency of its own data structures.
    *   **Exercisers:** The application contains built-in self-test code that it can run on its own algorithms to verify correctness.
    *   **Internal Monitoring:** The application actively reports its own health status, performance metrics, and progress to an external monitoring system.

***
#### 3.1.2 Responding to Failures: A Recovery Methodology

When monitoring detects an "unhealthy" process, the system must have a structured methodology for recovery.

*   **Identify and Isolate:** The first step is to determine which processes are affected. It's important to recognize that the *hung* process may not be the *broken* one; it may be the victim of another failed component. The goal is to kill and restart as few processes as possible to minimize disruption.

*   **Automated Restarts:** Applications must be designed to support different types of restarts.
    *   **Cold Restart:** Starting from scratch with no pre-existing state.
    *   **Warm Restart:** Resuming from a previously saved state or checkpoint.
    *   **Partial Restart:** Restarting only a subset of the system's components.

*   **Restart Groups:** In highly available, complex systems, processes are organized into **restart groups**. These are sets of interdependent processes that must be started, stopped, and managed as a single unit. The system also defines **inter-group dependencies** (e.g., restart database group B only after logging group A is fully operational).

*   **Escalation Mechanisms:** Recovery should follow an escalation path to minimize impact.
    1.  **Retry:** Attempt the failed operation again, but not forever, as this can hold resources and keep clients waiting indefinitely.
    2.  **Roll Back and Report Error:** If retries fail, undo any partial operations, release resources, and return a failure code to the client.
    3.  **Continue with Reduced Capacity:** If a component cannot be recovered, the system may continue to run with reduced functionality, handling the requests it can and rejecting others.
    4.  **Escalate Restarts:** If restarting a single group fails, the system should escalate to restarting larger sets of groups or, as a last resort, rebooting entire machines.

***
### 3.2 Related Problems That Health Monitoring Can Handle

A key advantage of health monitoring is its ability to detect a wide range of synchronization-related failures that are not formal deadlocks. A deadlock detection algorithm, which only looks for circular resource dependencies, would miss these issues entirely. However, they can cause a system to hang just as effectively.

*   **Livelock**
    *   **Definition:** A state where two or more processes are actively executing instructions but are not making any forward progress because they are continuously reacting to each other's state changes.
    *   **Example:** A process is running but will not release resource R1 until it receives a specific message. However, the process that is supposed to send that message is blocked, waiting to acquire R1. Both processes are active (one is spinning or repeatedly checking, the other is blocked), but the system is stuck in a dependency loop without a formal deadlock.

*   **Sleeping Beauty (Missed Wakeup)**
    *   **Definition:** A process is blocked indefinitely, awaiting an event or signal that will never arrive.
    *   **Example:** This commonly occurs due to race conditions, such as the classic **sleep/wakeup race**. A process checks a condition, decides to go to sleep, but before it can execute the sleep instruction, another process signals the wakeup. The wakeup signal is missed, and the first process goes to sleep forever, waiting for a "Prince Charming" that has already come and gone.

*   **Priority Inversion Hangs**
    *   **Definition:** A high-priority process is blocked waiting for a resource held by a low-priority process. The low-priority process never gets scheduled to run—because other, medium-priority processes are consuming the CPU—so it can never release the resource to unblock the high-priority one.
    *   **Example:** This was the root cause of the infamous software resets on the **Mars Pathfinder** mission.

None of these scenarios create a circular wait for resources and thus are not true deadlocks. However, from an external perspective, they all result in a hung system. Health monitoring, by focusing on symptoms like unresponsiveness or lack of progress, can successfully detect and enable recovery from all of these cases.

---
## 4.0 Simplifying Synchronization

Using low-level primitives like mutexes and semaphores is complex and error-prone. A programmer might forget to use a lock, use it incorrectly, or create a deadlock. To mitigate this, higher-level approaches aim to make synchronization easier by shifting the implementation burden from the programmer to the compiler and language runtime.

***
### 4.1 The Compiler-Driven Approach

This foundational approach simplifies synchronization by defining a clear separation of concerns:

1.  **The Programmer's Role: Identify Shared Resources.** The developer identifies which objects or classes represent shared resources whose methods may require serialized access.
2.  **Assume Serialization:** The developer writes the code to operate on these objects *without* writing explicit `lock()` or `unlock()` calls, simply assuming that critical sections will be correctly serialized.
3.  **The Compiler's Role: Generate Synchronization.** Based on keywords or annotations provided by the programmer, the compiler automatically generates the low-level synchronization code. This includes inserting lock acquisitions and releases in the correct places using appropriate, optimized mechanisms.

This model is the basis for constructs like Monitors and Java's Synchronized Methods.

***
### 4.2 Monitors: Protected Classes

A **monitor** is a high-level synchronization construct where an entire object is treated as a critical section.

*   **Mechanism:**
    *   A class is declared as a `monitor`.
    *   Each object instance of that class gets a single, implicit mutex.
    *   This mutex is **automatically acquired** at the start of *any* method invocation on the object and **automatically released** when the method returns.

*   **Example:**
    ```java
    monitor CheckBook {
        // This object is locked whenever ANY method is invoked.
        private int balance;

        public int balance() {
            return(balance);
        }

        public int debit(int amount) {
            balance -= amount;
            return(balance);
        }
    }
    ```
    In this example, calls to `balance()` and `debit()` on the same `CheckBook` object will never run concurrently.

*   **Simplicity vs. Performance (The TANSTAAFL Principle):**
    *   **Pros:** Monitors offer maximum simplicity and safety. The developer cannot forget a lock.
    *   **Cons:** This is very **coarse-grained locking**. It locks the entire object for the duration of every method, even if some methods are read-only or do not access shared state. This eliminates parallelism, which can create severe performance bottlenecks and convoys. This illustrates the principle of "There Ain't No Such Thing As A Free Lunch" (TANSTAAFL): the ease of use comes at a significant performance cost.

***
### 4.3 Java Synchronized Methods: Finer-Grained Control

Java provides a more flexible approach that allows the developer to specify which methods need protection.

*   **Mechanism:**
    *   Every Java object has an associated mutex (its "monitor lock").
    *   A method is made mutually exclusive by marking it with the `synchronized` keyword. The lock is only acquired for these specific methods.
    *   **Reentrancy:** Nested calls to synchronized methods *by the same thread* do not re-acquire the lock. The thread already holds it and can proceed without self-deadlocking.
    *   **Class-Level Locks:** A `static synchronized` method acquires a lock on the entire `class` object, not an individual instance, serializing calls across all instances of that class.

*   **Example:**
    ```java
    class CheckBook {
        private int balance;

        // NOT synchronized. Multiple threads can call this concurrently.
        public int balance() {
            return(balance);
        }

        // IS synchronized. Only one thread can execute this method
        // on a given CheckBook instance at a time.
        public synchronized int debit(int amount) {
            balance -= amount;
            return(balance);
        }
    }
    ```
    Here, `debit()` is protected, but `balance()` is not, allowing for more parallelism (though potentially exposing `balance()` to race conditions if not handled carefully).

*   **Pros and Cons:**
    *   **Pros:** Offers **finer lock granularity**, which can significantly improve performance and reduce the risk of deadlocks compared to a full monitor.
    *   **Cons:** This flexibility places the responsibility back on the developer to correctly identify *every* method that accesses shared state and needs to be serialized. An omission will result in subtle and hard-to-debug race conditions.

---
## 5.0 Conclusion

*   Parallelism is essential for achieving high performance in modern computing.
*   However, parallelism introduces the possibility of serious, non-deterministic errors, with deadlock being a primary example.
*   Programmers working with parallel code must understand the principles of synchronization, its associated problems (like deadlock), and the various solutions and their trade-offs in terms of correctness, performance, and complexity.
