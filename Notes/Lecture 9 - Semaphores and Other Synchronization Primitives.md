# Lecture 9: Semaphores and Other Synchronization Primitives

## 1. Introduction to Synchronization Primitives
Synchronization primitives are tools used to manage access to shared resources in a concurrent environment, ensuring correctness and preventing race conditions. This lecture covers semaphores, a foundational theoretical tool, as well as more practical mechanisms like mutexes and various locking strategies. It also explores the performance implications and common problems associated with locking, such as contention and priority inversion.

---
## 2. Semaphores

### 2.1. Definition and Origin
Semaphores are a synchronization mechanism introduced by computer scientist Edsger Dijkstra in 1968. They provide a theoretically sound way to implement locks and solve other synchronization problems. Because of their precise specification and early invention, they form the foundation for most theoretical studies in computational synchronization.

***
### 2.2. Core Components
A semaphore is a data structure composed of two main parts:
1.  **An Integer Counter:** This counter can hold any integer value and is initialized upon the semaphore's creation. Its value represents the state of the resource or condition being managed.
2.  **A FIFO Waiting Queue:** A First-In, First-Out queue that holds processes or threads that are blocked, waiting for the semaphore to become available.

***
### 2.3. Operations
There are two atomic operations that can be performed on a semaphore. The names originate from the Dutch language.

1.  **P (proberen, "to test") or `wait()`:** This operation is used to acquire a resource or wait for a condition.
    *   It decrements the semaphore's integer counter by 1.
    *   It then checks the counter's value:
        *   If `count >= 0`, the process continues execution, having successfully acquired the resource.
        *   If `count < 0`, the process is placed at the end of the FIFO waiting queue and is blocked (put to sleep).

2.  **V (verhogen, "to raise") or `post()`/`signal()`:** This operation is used to release a resource or signal that an event has occurred.
    *   It increments the semaphore's integer counter by 1.
    *   It then checks the waiting queue:
        *   If the queue is not empty, it wakes up the first process in the queue, moving it from the blocked state to the ready state.
        *   If the queue is empty, the operation simply completes.

***
### 2.4. Use Cases and Examples

***
#### 2.4.1. Mutual Exclusion (Binary Semaphores)
Semaphores can function as locks to ensure mutual exclusion for a critical section. In this role, they are often called "binary semaphores."

*   **Initialization:** The semaphore counter is initialized to `1`, indicating that one thread is allowed to enter the critical section.
*   **Acquiring the Lock:** A thread calls the `P()` operation before entering the critical section.
    *   The first thread to call `P()` decrements the count from 1 to `0`. Since `0 >= 0`, it proceeds.
    *   Any subsequent thread calling `P()` will decrement the count to `-1` (or lower). Since the count is negative, the thread blocks and is added to the queue.
*   **Releasing the Lock:** The thread calls the `V()` operation after leaving the critical section.
    *   This increments the count (e.g., from `-1` to `0`).
    *   Because the waiting queue is not empty, the first thread in the queue is woken up and can now enter the critical section.

**Example: `write_check` function**
The following code snippet demonstrates using a semaphore to protect a bank account balance during a withdrawal.

```c
struct account {
    struct semaphore s; /* Initialized to count=1, queue=empty */
    int balance;
    ...
};

int write_check(struct account *a, int amount) {
    int ret;

    // Acquire exclusive access to the account
    wait(&a->semaphore); // P() operation

    // --- CRITICAL SECTION START ---
    if (a->balance >= amount) {
        a->balance -= amount;
        ret = amount;
    } else {
        ret = -1;
    }
    // --- CRITICAL SECTION END ---

    // Release access to the account
    post(&a->semaphore); // V() operation

    return(ret);
}
```

***
#### 2.4.2. Notifications and Asynchronous Waits
Semaphores can be used to signal the completion of events, allowing one or more threads to wait for work to become available.

*   **Initialization:** The semaphore counter is initialized to `0`, reflecting that no events have been completed or no work is available.
*   **Awaiting Work:** A worker thread calls `P()` to wait for work.
    *   If the count is `0`, it will be decremented to `-1`, and the worker thread will block.
*   **Signaling Work:** When a new piece of work is ready, a producer thread calls `V()`.
    *   This increments the count (e.g., from `-1` to `0`).
    *   If a worker was blocked, it is awakened to process the work. If no workers were waiting, the count is incremented (e.g., to `1`), indicating that one unit of work is pending.

| Action | Count Before | Count After | State of Worker |
| :--- | :--- | :--- | :--- |
| Worker 1 calls `P()` (no work) | 0 | -1 | Blocked |
| Worker 2 calls `P()` (no work) | -1 | -2 | Blocked |
| Producer calls `V()` (work arrives) | -2 | -1 | Worker 1 is unblocked |
| Producer calls `V()` (work arrives) | -1 | 0 | Worker 2 is unblocked |
| Producer calls `V()` (work arrives) | 0 | 1 | No workers waiting |
| Worker 3 calls `P()` (work pending) | 1 | 0 | Proceeds immediately |

***
#### 2.4.3. Counting Semaphores for Resource Pools
Semaphores can manage access to a finite pool of identical resources (e.g., database connections, hardware devices).

*   **Initialization:** The semaphore counter is initialized to `N`, the number of available resources.
*   **Consuming a Resource:** A thread calls `P()` to request a resource.
    *   As long as the count is positive after being decremented, the thread can take a resource and proceed.
    *   Once all resources are in use (count becomes `0`), subsequent threads calling `P()` will block.
*   **Releasing a Resource:** When a thread is finished with a resource, it calls `V()`, incrementing the count and potentially waking a waiting thread.

***
### 2.5. Limitations of Semaphores
Despite their theoretical importance, semaphores have practical limitations:
*   **Simplicity:** They offer few features beyond the basic P and V operations.
*   **No Non-Blocking Check:** It is not possible to check if a lock is held without also attempting to acquire it, which may lead to blocking.
*   **No Reader/Writer Support:** They do not natively support the common reader/writer lock pattern, where multiple concurrent readers are allowed but writers require exclusive access.
*   **No Recovery from Wedged V Operation:** If a process holding a semaphore crashes before calling `V()`, the semaphore remains locked, and any processes waiting on it will wait forever.
*   **Priority Scheduling Issues:** They do not inherently handle priority inversion, a problem that can arise in priority-based scheduling systems.

---
## 3. Practical Locking Mechanisms and Problems

### 3.1. Mutexes
A **mutex** (mutual exclusion) is a common locking primitive found in systems like Linux and Unix, especially in threaded applications.
*   **Purpose:** To protect a critical section of code.
*   **Scope:** Typically used by multiple threads within the same process, operating in a single address space.
*   **Characteristics:** They are designed to be held for brief durations (nanoseconds to milliseconds) and are generally implemented with low overhead.

***
### 3.2. Object-Level Locking
When dealing with persistent objects like files, which may be accessed by multiple independent processes and held for long durations, locking the code (with a mutex) is insufficient. Instead, the object itself is locked.
*   **Challenges:**
    *   Critical sections are longer.
    *   Access is from different programs and address spaces.
    *   Access may originate from different computers in a distributed system.
*   **Solution:** Object-specific locking mechanisms, often provided by the operating system.

***
### 3.3. Advisory vs. Enforced Locking

*   **Advisory Locking:** A cooperative locking convention. Processes must voluntarily check for and respect the lock. The system does not prevent a non-cooperating process from accessing the locked resource. This offers flexibility but is error-prone.
    *   *Examples:* Mutexes, `flock()` in Linux.
*   **Enforced Locking:** The system itself enforces the lock. Any attempt to access the locked resource is blocked by the access mechanism (e.g., the operating system), regardless of whether the process is "cooperating." This guarantees protection but can be less flexible.

***
### 3.4. Linux File Locking Examples
Linux provides several mechanisms for file locking.

| Mechanism | `flock(fd, operation)` | `lockf(fd, cmd, offset, len)` |
| :--- | :--- | :--- |
| **Type** | Advisory | Can be enforced |
| **Scope** | Locks an open file descriptor (`fd`). Only affects threads in the same process or processes that inherited the `fd`. | Locks the file itself. Affects all processes on the system trying to access that file. |
| **Granularity**| Locks the entire file. | Can lock a specific byte range within the file. |
| **Enforcement**| Never enforced by the kernel. | Enforcement depends on the underlying filesystem implementation. |
| **Operations** | `LOCK_SH` (shared), `LOCK_EX` (exclusive), `LOCK_UN` (unlock) | `F_LOCK` (exclusive), `F_ULOCK` (unlock), `F_TEST` (non-blocking test) |

---
## 4. Problems with Locking

### 4.1 Performance and Costs of Locking

Locking is a fundamental tool for ensuring correctness in concurrent systems, but it is not without cost. The overhead associated with locking can significantly impact system performance. This cost can be broken down into two primary categories: the fixed execution overhead of the locking mechanism itself, and the much larger, variable cost incurred when contention leads to blocking.

***
#### 4.1.1 Sources of Locking Overhead

The performance penalty of locking arises from the extra work the system must do to manage the lock state.

1.  **Execution Overhead (The "Get" Cost):** This is the baseline cost paid for every lock and unlock operation, even when there is no contention (i.e., the lock is immediately available). The magnitude of this cost depends on the implementation:
    *   **OS System Call:** When locking is enforced by the operating system (common for inter-process locking like `lockf`), acquiring a lock requires a system call. This is a high-overhead operation involving a context switch from user mode to kernel mode, execution of kernel code to manage the lock, and a subsequent context switch back to user mode.
    *   **User-Level Library:** When locking is managed within a user-level library (common for mutexes used by threads in the same process), the overhead is lower. It is a standard function call within the process's address space. However, it is not free; it still requires executing the instructions within the lock/unlock routines to check and update the lock's state.

2.  **Blocking Overhead (The "Block" Cost):** This is a substantial additional cost incurred only when there is **contention**—that is, when a thread attempts to acquire a lock that is already held by another thread. Blocking is significantly more expensive than a successful lock acquisition (potentially by a factor of 1000x or more) because it involves a sequence of OS-level actions:
    *   The thread's state is changed from *running* to *blocked*.
    *   The OS must record why the thread is blocked (i.e., which lock it is waiting for).
    *   The thread is removed from the scheduler's consideration and placed onto a specific wait queue associated with the lock.
    *   The OS scheduler must perform a full context switch to select and run a different ready thread.
    *   Later, when the lock is released, the OS must identify the waiting thread, move it from the wait queue back to the ready queue, and make it eligible for scheduling again.

***
#### 4.1.2 The Cost-Benefit Imbalance
A critical performance consideration is that the overhead of locking can often be much greater than the execution time of the code it is protecting.
*   **Critical Section Time:** Many critical sections are extremely brief, consisting of only a handful of instructions that execute in nanoseconds (e.g., `counter++`).
*   **Locking Time:** The process of acquiring and releasing the lock, even without contention, can take microseconds due to function call overhead and memory access patterns.
*   **Implication:** In such cases, the program spends orders of magnitude more time managing the lock than performing the actual work within the critical section. This highlights why minimizing the frequency of locking and the size of critical sections is crucial for performance.

***
#### 4.1.3 The Role of Conflict Probability
The actual average cost a system pays for locking is a weighted average of the "get" cost and the "block" cost, determined by the probability of contention.

*   **Conflict Probability (P_conflict):** This is the likelihood that a thread attempting to acquire a lock will find it already held by another thread. This probability is the primary driver of synchronization-related performance degradation.

The expected cost of a single lock attempt can be modeled by the following formula:

> **C_expected = (C_block * P_conflict) + (C_get * (1 - P_conflict))**

Where:
*   `C_expected` is the average cost of a lock attempt.
*   `C_block` is the very high cost of blocking due to contention.
*   `C_get` is the baseline cost of acquiring an uncontested lock.
*   `P_conflict` is the probability of conflict.
*   `(1 - P_conflict)` is the probability of success without contention.

Because `C_block` is so much larger than `C_get`, system performance is extremely sensitive to `P_conflict`. Even a small increase in the probability of contention can cause the average cost of synchronization to rise dramatically, leading directly to performance issues like convoy formation and resource bottlenecks.

***
### 4.2. Contention and Convoy Formation

**Contention** occurs when multiple threads simultaneously compete for the same lock. High and persistent contention leads to **convoy formation**: a stable, self-sustaining queue of blocked threads waiting for a single resource, which severely degrades system throughput.

***
#### 4.2.1. Dynamics and Consequences

A convoy forms when new threads join the wait queue for a lock faster than the current lock-holder can complete its task and release it. This creates a positive feedback loop where the queue grows, solidifying the convoy.

The primary consequences are:
*   **Elimination of Parallelism:** A convoy serializes execution. Only one thread makes progress at a time, negating the benefits of multi-core hardware.
*   **Bottleneck Creation:** The contended lock becomes a system bottleneck, limiting the throughput of all involved threads to the execution speed of a single serialized task.
*   **Throughput Cliff:** Once a convoy forms, system throughput collapses. Adding more work or threads does not improve performance; it only lengthens the convoy.

***
#### 4.2.2. Mathematical Models of Contention

The probability of conflict (`P_conflict`) can be modeled to show how contention arises.

*   **General Conflict Probability:** This formula calculates the chance that a thread requesting a lock will find it busy.
    > **P<sub>conflict</sub> = 1 - (1 - (T<sub>critical</sub> / T<sub>total</sub>))<sup>threads</sup>**

    This model shows that the probability of conflict is 1 minus the probability that *no thread* currently holds the lock. Contention rises exponentially with the number of threads and the fraction of time (`T_critical` / `T_total`) spent holding the lock.

    ![alt text](https://i.imgur.com/AM9yoKT.png "Probability of Conflict relative to Critical Section Time")

*   **The Convoy Feedback Loop:** Once a queue forms, the waiting time (`T_wait`) itself extends the period the resource is unavailable, worsening the problem.
    > **P<sub>conflict</sub> = 1 - (1 - ((T<sub>wait</sub> + T<sub>critical</sub>) / T<sub>total</sub>))<sup>threads</sup>**

    Here, an initial `T_wait` increases `P_conflict`, which in turn leads to longer queues and a larger `T_wait`, creating the feedback loop that sustains the convoy.

*   **The Tipping Point:** A convoy becomes effectively permanent when the queue's growth outpaces its service rate.
    > If **`T_wait` ≥ Mean Inter-Arrival Time**, parallelism ceases.

    This condition means a new thread arrives and joins the queue before the previously arrived thread has even been served. The system is now locked into a purely serial execution pattern for all threads in the convoy.
    
     ![alt text](https://i.imgur.com/2RLKhaq.png "Tipping Point Graph")

***
### 4.3. Priority Inversion

Priority inversion is a hazardous synchronization problem that occurs in systems using **preemptive, priority-based scheduling** in conjunction with locking mechanisms. It undermines the core principle of a priority scheduler, allowing a low-priority task to indirectly block a high-priority task for an unbounded period of time.

***
#### 4.3.1. Definition and Consequence

At its core, priority inversion occurs when a high-priority task must wait for a resource (e.g., a mutex) that is currently held by a low-priority task. The problem is compounded when a **medium-priority task**, which does not need the resource, preempts the low-priority task and begins executing.

The dangerous result is that the high-priority task is not just waiting for the short time it would take the low-priority task to finish its critical section; it is now effectively blocked by the execution of the medium-priority task. Its execution priority has been "inverted" below that of a medium-priority task. This can lead to missed deadlines in real-time systems, poor system responsiveness, and in the worst cases, total system failure.

***
#### 4.3.2. The "Ingredients" for Priority Inversion

For priority inversion to occur, a specific set of conditions must be met:
1.  A **preemptive priority-based scheduler** must be in use.
2.  There must be at least three tasks (or threads) with distinct priority levels: **Low**, **Medium**, and **High**.
3.  There must be a shared resource protected by a lock that at least the Low and High priority tasks need to access.
4.  A specific sequence of events must unfold:
    *   The Low-priority task acquires the lock.
    *   The High-priority task preempts the Low-priority task but then blocks when it attempts to acquire the same lock.
    *   The Medium-priority task becomes ready to run and, having a higher priority than the now-preempted Low-priority task, is scheduled to run.

***
#### 4.3.3. The Mars Pathfinder Case Study: A Real-World Example

A famous instance of this problem occurred on the Mars Pathfinder rover, which used the VxWorks real-time operating system. This issue caused periodic, difficult-to-diagnose system reboots.

*   **The Players:** The system had numerous tasks, but three were key to the failure.

| Task Name | Priority | Lock Usage | Run Duration |
| :--- | :--- | :--- | :--- |
| **Bus Management** | **High (P1)** | Needs bus mutex | Short |
| **Communications** | **Medium (P2)** | Does **not** need mutex| Long |
| **Meteorological**| **Low (P3)** | Needs bus mutex | Short |

*   **The Failure Scenario (Step-by-Step):**

    1.  The `Low-priority` meteorological task (P3) begins to run and successfully acquires the mutex for the shared information bus.
    2.  The `High-priority` bus management task (P1) becomes ready. Due to its high priority, it immediately preempts P3.
    3.  P1 attempts to acquire the bus mutex to do its work. However, the mutex is held by P3, so P1 **blocks**, waiting for P3 to release it.
    4.  The `Medium-priority` communications task (P2) becomes ready to run. The scheduler must now choose between the ready tasks: P2 and P3.
    5.  Since P2 has a higher priority than P3, the scheduler chooses to run P2.
    6.  **The Inversion:** P2, a long-running task, now executes. Because it is running, the low-priority P3 is starved and cannot run to finish its critical section and release the mutex.
    7.  This means the high-priority P1 remains blocked. **Effectively, the medium-priority task (P2) is blocking the high-priority task (P1).**

*   **The Ultimate Effect:** The system had a watchdog timer. When this watchdog saw that the critical, high-priority bus management task had not run for a long time, it concluded that a fatal error had occurred and initiated a full system reboot.

***
#### 4.3.4. The Solution: Priority Inheritance

Priority inversion is a well-understood problem with a standard solution known as **priority inheritance**.

*   **The Concept:** When a high-priority task blocks while trying to acquire a lock held by a low-priority task, the system **temporarily boosts the low-priority task's priority to match that of the high-priority task**.

*   **Applying the Fix to Pathfinder:**
    1.  P1 (High) blocks on the mutex held by P3 (Low).
    2.  **The Fix:** The OS immediately elevates P3's priority to be equal to P1's (High).
    3.  Now, when P2 (Medium) becomes ready, the scheduler compares the priorities of P2 and P3. P3, with its temporarily boosted priority, is now higher than P2.
    4.  P3 is scheduled to run immediately. It quickly finishes its short critical section and releases the mutex.
    5.  The moment P3 releases the mutex, its priority is demoted back to its original Low level.
    6.  P1 is unblocked and, as the highest-priority ready task, acquires the mutex and runs without delay.

By temporarily elevating the priority of the lock-holding task, priority inheritance ensures that the critical section is completed as quickly as possible, minimizing the time the high-priority task is blocked. This solution was applied to the Mars Pathfinder remotely by changing a single configuration flag in VxWorks, which completely solved the system reset problem.

---
### 5. Strategies for Solving Locking Problems

Effective lock management focuses on reducing contention, not on reimplementing the highly-optimized primitives provided by the OS. The goal is to minimize the performance impact of locks by changing how and when they are used.

***
#### 5.1. Eliminate the Critical Section Entirely
The most effective strategy is to remove the need for a lock altogether.
*   **How:**
    *   **Use Private Resources:** Redesign algorithms to use thread-local data, aggregating results only at the end.
    *   **Employ Atomic Instructions:** For simple operations like increments, use hardware atomic instructions (`FETCH_AND_ADD`, `COMPARE_AND_SWAP`) instead of locks.
    *   **Utilize Lock-Free Data Structures:** Use advanced data structures designed for concurrent access without locks.
*   **Trade-offs:** This approach often requires a fundamental redesign of the software. Lock-free programming is highly complex and error-prone but offers the highest possible performance.

***
#### 5.2. Eliminate Preemption During Critical Section
A thread holding a lock cannot cause contention if it cannot be preempted. This ensures the lock is released as quickly as possible.
*   **How:**
    *   **Disable Interrupts:** On a single-core system, disabling interrupts prevents the scheduler from running, guaranteeing the current thread continues execution. This is a privileged operation and is generally not feasible or safe for user-level applications.
    *   **Avoid Blocking Operations:** Do not perform any action that could yield control to the scheduler while holding a lock. This includes explicit `yield()` calls and, most importantly, any form of I/O or system call that might block.
*   **Trade-offs:** Disabling interrupts is a dangerous, system-wide action that can harm overall system responsiveness. It is not an option in multi-core environments to prevent parallel execution. This strategy is therefore very limited in modern systems.

***
#### 5.3. Reduce Time Spent in Critical Section
Minimize lock holding time (`T_critical`) to decrease the window for potential conflicts.
*   **How:**
    *   **Isolate Race-Prone Code:** The `lock()`/`unlock()` block should contain only the absolute minimum code necessary to prevent a race condition.
    *   **Move Slow Operations:** Execute any slow or blocking operations (e.g., I/O, memory allocation, complex calculations) *before* acquiring the lock or *after* releasing it.
*   **Trade-offs:** Increases code complexity by splitting a single logical operation into pre-lock, locked, and post-lock phases.

***
#### 5.4. Reduce Frequency of Entering Critical Section
Acquire locks less often to reduce the number of opportunities for contention.
*   **How:**
    *   **Batching:** Group multiple updates into a single locked operation. Instead of locking 100 times to add 100 items, lock once to add a batch of 100.
    *   **Sloppy Counters:** Each thread updates a private, unlocked counter. Periodically, it acquires a global lock to merge its local total, then resets its private counter. This dramatically reduces contention by turning many expensive global lock operations into cheap local updates. The trade-off is that the global counter is not perfectly accurate in real-time, as it lags behind the sum of the private counts.
*   **Trade-offs:** Introduces latency and data staleness (the global state is not always up-to-date). A thread failure can result in lost updates.

#### 5.5. Reduce Exclusive Use of the Serialized Resource
For data that is read far more than written, allow multiple concurrent readers to reduce contention.
*   **How:** A read/write lock provides two modes:
    *   **Read Lock (Shared):** An unlimited number of threads can hold a read lock simultaneously, as long as no write lock is held.
    *   **Write Lock (Exclusive):** Only one thread can hold a write lock, and only if no other locks (read or write) are held.
*   **Trade-offs:** Can lead to **writer starvation** if a constant stream of readers prevents a writer from ever acquiring the lock. The implementation requires a fairness policy to mitigate this.

***
#### 5.6. Spread Requests Over More Resources
This strategy involves adjusting the scope of data protected by a single lock to reduce contention.

*   **Coarse-Grained Locking:** A single lock protects a large resource (e.g., a "pool" of buffers). This is simple to implement but creates a major bottleneck, as all threads must compete for one lock.
*   **Fine-Grained Locking:** Each component ("element") of a resource has its own lock. This allows high parallelism, as different threads can access different elements concurrently.

A common hybrid pattern uses fine-grained locks for element access but also requires a **coarse-grained pool lock** for operations that modify the entire collection's structure (e.g., adding or removing an element). This necessary pool lock can become a bottleneck, serializing access. To mitigate this:
1.  **Minimize its use** by designing algorithms that rarely need to lock the whole pool.
2.  **Use a reader/writer lock** for the pool to allow concurrent scanning.
3.  **Partition into sub-pools**, each with its own lock, to spread contention.

---
## 6. Conclusion and The Next Problem: Deadlock
Careful use of synchronization primitives like locks and semaphores can solve concurrency issues and their performance can be managed. However, the use of locks introduces a new, severe risk: **deadlock**. This is a state where two or more processes are blocked forever, each waiting for a resource held by another, leading to a complete system freeze.
