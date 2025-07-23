# CS 111: Semaphores and Other Synchronization Primitives

## 1. Introduction to Synchronization Primitives
Synchronization primitives are tools used to manage access to shared resources in a concurrent environment, ensuring correctness and preventing race conditions. This lecture covers semaphores, a foundational theoretical tool, as well as more practical mechanisms like mutexes and various locking strategies. It also explores the performance implications and common problems associated with locking, such as contention and priority inversion.

## 2. Semaphores

### 2.1. Definition and Origin
Semaphores are a synchronization mechanism introduced by computer scientist Edsger Dijkstra in 1968. They provide a theoretically sound way to implement locks and solve other synchronization problems. Because of their precise specification and early invention, they form the foundation for most theoretical studies in computational synchronization.

### 2.2. Core Components
A semaphore is a data structure composed of two main parts:
1.  **An Integer Counter:** This counter can hold any integer value and is initialized upon the semaphore's creation. Its value represents the state of the resource or condition being managed.
2.  **A FIFO Waiting Queue:** A First-In, First-Out queue that holds processes or threads that are blocked, waiting for the semaphore to become available.

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

### 2.4. Use Cases and Examples

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

#### 2.4.3. Counting Semaphores for Resource Pools
Semaphores can manage access to a finite pool of identical resources (e.g., database connections, hardware devices).

*   **Initialization:** The semaphore counter is initialized to `N`, the number of available resources.
*   **Consuming a Resource:** A thread calls `P()` to request a resource.
    *   As long as the count is positive after being decremented, the thread can take a resource and proceed.
    *   Once all resources are in use (count becomes `0`), subsequent threads calling `P()` will block.
*   **Releasing a Resource:** When a thread is finished with a resource, it calls `V()`, incrementing the count and potentially waking a waiting thread.

### 2.5. Limitations of Semaphores
Despite their theoretical importance, semaphores have practical limitations:
*   **Simplicity:** They offer few features beyond the basic P and V operations.
*   **No Non-Blocking Check:** It is not possible to check if a lock is held without also attempting to acquire it, which may lead to blocking.
*   **No Reader/Writer Support:** They do not natively support the common reader/writer lock pattern, where multiple concurrent readers are allowed but writers require exclusive access.
*   **No Recovery from Wedged V Operation:** If a process holding a semaphore crashes before calling `V()`, the semaphore remains locked, and any processes waiting on it will wait forever.
*   **Priority Scheduling Issues:** They do not inherently handle priority inversion, a problem that can arise in priority-based scheduling systems.

## 3. Practical Locking Mechanisms and Problems

### 3.1. Mutexes
A **mutex** (mutual exclusion) is a common locking primitive found in systems like Linux and Unix, especially in threaded applications.
*   **Purpose:** To protect a critical section of code.
*   **Scope:** Typically used by multiple threads within the same process, operating in a single address space.
*   **Characteristics:** They are designed to be held for brief durations (nanoseconds to milliseconds) and are generally implemented with low overhead.

### 3.2. Object-Level Locking
When dealing with persistent objects like files, which may be accessed by multiple independent processes and held for long durations, locking the code (with a mutex) is insufficient. Instead, the object itself is locked.
*   **Challenges:**
    *   Critical sections are longer.
    *   Access is from different programs and address spaces.
    *   Access may originate from different computers in a distributed system.
*   **Solution:** Object-specific locking mechanisms, often provided by the operating system.

### 3.3. Advisory vs. Enforced Locking

*   **Advisory Locking:** A cooperative locking convention. Processes must voluntarily check for and respect the lock. The system does not prevent a non-cooperating process from accessing the locked resource. This offers flexibility but is error-prone.
    *   *Examples:* Mutexes, `flock()` in Linux.
*   **Enforced Locking:** The system itself enforces the lock. Any attempt to access the locked resource is blocked by the access mechanism (e.g., the operating system), regardless of whether the process is "cooperating." This guarantees protection but can be less flexible.

### 3.4. Linux File Locking Examples
Linux provides several mechanisms for file locking.

| Mechanism | `flock(fd, operation)` | `lockf(fd, cmd, offset, len)` |
| :--- | :--- | :--- |
| **Type** | Advisory | Can be enforced |
| **Scope** | Locks an open file descriptor (`fd`). Only affects threads in the same process or processes that inherited the `fd`. | Locks the file itself. Affects all processes on the system trying to access that file. |
| **Granularity**| Locks the entire file. | Can lock a specific byte range within the file. |
| **Enforcement**| Never enforced by the kernel. | Enforcement depends on the underlying filesystem implementation. |
| **Operations** | `LOCK_SH` (shared), `LOCK_EX` (exclusive), `LOCK_UN` (unlock) | `F_LOCK` (exclusive), `F_ULOCK` (unlock), `F_TEST` (non-blocking test) |

## 4. Problems with Locking

### 4.1. Performance Overhead
Locking is not free. It introduces overhead that can impact performance.
*   **Execution Cost:** Acquiring and releasing a lock requires executing instructions, either in a library or via a more expensive system call. This overhead can be significant, especially if the critical section is very short.
*   **Blocking Cost:** If a thread fails to acquire a lock, it blocks. Blocking is much more expensive, involving context switches and scheduler activity. The expected cost of a lock operation depends on the probability of conflict:
    `C_expected = (C_block * P_conflict) + (C_get * (1 - P_conflict))`

### 4.2. Contention and Convoy Formation
When a resource is highly contested, multiple threads frequently try to acquire its lock simultaneously.
*   **Convoy:** A long queue of blocked threads forms, waiting for the single lock.
*   **Effect:** Parallelism is eliminated for those threads. The shared resource becomes a **bottleneck**, and overall system throughput can drastically decrease, as shown in the performance graph where throughput falls off a cliff once a convoy forms.
*   **Probability of Conflict:** The likelihood of contention increases with the number of threads and the fraction of time each thread spends in the critical section.

### 4.3. Priority Inversion
Priority inversion is a serious problem in preemptive, priority-based scheduling systems that also use locks.
*   **Definition:** A high-priority task becomes blocked, waiting for a resource held by a low-priority task. Meanwhile, a medium-priority task, which does not need the resource, preempts the low-priority task and runs. This effectively means the high-priority task is indefinitely blocked by a medium-priority task.

#### 4.3.1. The Mars Pathfinder Case Study
A real-world priority inversion occurred on the Mars Pathfinder rover, causing periodic system resets.
*   **The Tasks:**
    1.  **High-Priority (P1):** A frequent, short bus management task that needed a mutex for a shared "information bus."
    2.  **Low-Priority (P3):** An occasional, short meteorological task that also needed the bus mutex.
    3.  **Medium-Priority (P2):** A long-running communications task that did not need the mutex.
*   **The Failure Scenario:**
    1.  The low-priority meteorological task (P3) acquired the mutex.
    2.  The high-priority bus management task (P1) preempted P3 but blocked immediately when it tried to acquire the same mutex.
    3.  The medium-priority communications task (P2) became ready to run. Since P1 was blocked and P2 had higher priority than P3, the scheduler ran P2.
    4.  P2 ran for a long time, preventing P3 from ever running to release the mutex.
*   **The Result:** P1 was starved. A watchdog timer, noticing that the critical P1 task hadn't run, would assume a fatal error and reboot the entire system.

#### 4.3.2. The Solution: Priority Inheritance
Priority inversion is a well-known problem with a standard solution.
*   **Priority Inheritance:** When a high-priority task (P1) blocks on a lock held by a low-priority task (P3), the system temporarily boosts P3's priority to that of P1.
*   **Effect:** This allows P3 to run immediately (preempting any medium-priority tasks like P2), finish its critical section, and release the lock. Once the lock is released, P3's priority reverts to normal, and P1 can acquire the lock and proceed.

## 5. Strategies for Solving Locking Problems

### 5.1. Reducing Contention
Several strategies can be employed to mitigate lock contention and its performance impact.

1.  **Eliminate the Critical Section:** The best solution, if possible. This can be done by giving each thread a private copy of the resource or by using lock-free data structures and atomic instructions.
2.  **Reduce Time in Critical Section:** Keep critical sections as short as possible. Move all work that doesn't strictly require the lock (e.g., memory allocation, I/O, complex calculations) outside the locked region.
3.  **Reduce Frequency of Entering Critical Section:** Use the lock less often. This can be achieved through **batching** (grouping multiple updates into one) or with techniques like **sloppy counters**, where threads update local counters and only periodically lock a global counter to merge their results.
4.  **Use Read/Write Locks:** If a resource is read much more often than it is written, use a read/write lock. This allows multiple concurrent readers, reducing contention. A policy must be in place to prevent writer starvation.
5.  **Change Lock Granularity:**
    *   **Coarse-Grained Locking:** One lock for a large resource (e.g., a whole data structure). Simple to implement but causes high contention.
    *   **Fine-Grained Locking:** Multiple locks for different parts of a resource (e.g., one lock per node in a linked list). Reduces contention but is more complex, has higher overhead, and is more error-prone.

## 6. Conclusion and The Next Problem: Deadlock
Careful use of synchronization primitives like locks and semaphores can solve concurrency issues and their performance can be managed. However, the use of locks introduces a new, severe risk: **deadlock**. This is a state where two or more processes are blocked forever, each waiting for a resource held by another, leading to a complete system freeze.
