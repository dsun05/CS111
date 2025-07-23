# Lecture 8: Mutual Exclusion and Asynchronous Completion

## 1.0 Mutual Exclusion

Mutual exclusion is a synchronization principle that ensures only one process or thread can access a shared resource or execute a specific piece of code at a time. This prevents data corruption and inconsistent states that can arise from concurrent access.

***
### 1.1 Critical Sections

A **critical section** is a segment of code that accesses a shared resource (like a data structure or hardware device) and must not be executed by more than one thread or process simultaneously.

*   **Problem:** When multiple threads execute a critical section concurrently, their operations can interleave in an uncontrolled manner. This can lead to a state where each thread has only partially completed its task, resulting in data corruption or incorrect program behavior.
*   **Solution:** The problem is preventable by enforcing mutual exclusion, which guarantees that only one thread can be inside the critical section at any given time. If one thread is executing the section, all other threads attempting to enter must wait.

***
### 1.2 Contexts for Critical Sections

Critical sections are common in various computing scenarios:

*   **Multithreaded Applications:** Threads within the same process often share data structures. Any code that modifies these shared structures is a critical section.
*   **Processes:** Even independent processes can encounter critical sections when they share operating system resources, such as files or related data structures stored on disk.
*   **Avoiding Shared Resources:** Problems with mutual exclusion can be completely avoided if no resources are shared. However, this is often not feasible in modern applications, which rely on sharing for efficiency and functionality.

***
### 1.3 Recognizing Critical Sections

A code segment is likely a critical section if it exhibits the following characteristics:

1.  **Updates to Object State:** It involves modifying the state of one or more shared objects.
2.  **Multi-Step Operations:** The update requires multiple instructions to complete. During the execution of these steps, the shared object's state may be temporarily inconsistent.
3.  **Preemption Risk:** If a thread is preempted (e.g., due to a scheduler's clock interrupt) in the middle of the operation, the shared object is left in an inconsistent state, which can be seen by other threads.
4.  **Mutual Exclusion Requirement:** Correct operation demands that only one thread has access to the object(s) at a time, ensuring one client's operation completes fully before another's begins.

***
### 1.4 Atomicity

By using mutual exclusion, we can achieve **atomicity** for a critical section. Atomicity ensures that a sequence of operations is perceived as a single, indivisible operation. It has two main aspects:

1.  **Before-or-After Atomicity:** If two threads (A and B) attempt to enter a critical section, one will execute it completely before the other begins. There is no overlap. Either A enters and completes before B starts, or B enters and completes before A starts.
2.  **All-or-None Atomicity:** An operation within the critical section either completes successfully in its entirety, or it has no effect at all. From an external observer's perspective, an incomplete update is never visible; the system state is as if the update never began.

Correctness in concurrent systems generally requires both aspects of atomicity.

***
### 1.5 Options for Protecting Critical Sections

Several techniques can be used to protect critical sections and enforce mutual exclusion.

1.  **Turn Off Interrupts:**
    *   **Mechanism:** A thread disables all hardware interrupts before entering a critical section and re-enables them upon exit. This prevents the scheduler's clock interrupt from preempting the thread.
    *   **Drawbacks:** This is a privileged operation not available to user-level applications. It is also inefficient as it halts all other concurrent activities on the CPU, even unrelated ones, severely limiting system performance.

2.  **Avoid Shared Data:**
    *   **Mechanism:** Design the system so that threads or processes do not need to share data.
    *   **Advantages:** This is the simplest and most effective solution when feasible.
    *   **Limitations:** It is often not practical, as many applications fundamentally require data sharing. Even sharing read-only data is safe, as the order of reads does not matter. However, a single write operation to that data introduces the need for synchronization.

3.  **Hardware Mutual Exclusion (Atomic Instructions):**
    *   **Mechanism:** Utilize special CPU instructions that are guaranteed by the hardware to be atomic.
    *   **Atomicity of Instructions:** A single CPU instruction is uninterruptible. It executes on a "before-or-after" and "all-or-none" basis.
    *   **Capabilities:**
        *   **Read-Modify-Write:** These instructions can read a value from memory, modify it, and write it back in a single, atomic step. They typically operate on 1 to 8 bytes.
        *   **Simple Operations:** `increment`, `decrement`, `and`, `or`, `xor`.
        *   **Complex Operations:** `test-and-set`, `exchange`, `compare-and-swap`.
    *   **Limitations:** While some simple data structure updates can be designed to fit into a single atomic instruction, this is generally not feasible for complex critical sections. It is a highly specialized and difficult technique.

4.  **Software Locking:**
    *   **Mechanism:** Use a software data structure called a **lock** (or **mutex**) to guard a critical section. A thread must "acquire" or "hold" the lock before entering the critical section and "release" it upon exiting.
    *   **Operation:**
        *   A party needing to enter the critical section first tries to acquire the associated lock.
        *   If the lock is acquired successfully, the party proceeds.
        *   If the lock is held by another party, the requesting party must wait.
        *   After finishing its work in the critical section, the party releases the lock, allowing another waiting party to acquire it.

***
### 1.6 Building and Using Software Locks

Since Instruction Set Architectures (ISAs) do not typically provide a single "lock" instruction, locks must be constructed in software.

***
#### 1.6.1 The "Chicken-and-Egg" Problem of Lock Implementation

The operation of acquiring a lock is itself a critical section. To acquire a lock, a thread must check if it is free and, if so, mark it as taken. This "check-then-set" sequence involves multiple steps. If two threads execute this sequence concurrently, both might see the lock as free and mistakenly believe they have acquired it. This requires the lock acquisition process itself to be atomic.

#### 1.6.2 Hardware Assistance for Building Locks

This circular problem is solved by using the atomic CPU instructions provided by hardware designers. These instructions can perform the "check-then-set" operation in a single, uninterruptible step.

*   **Test-and-Set Instruction:**
    *   This is a C-like description of a single, atomic hardware instruction (`TS`).
    ```c
    // This is a C description of a single machine language instruction.
    // Real instructions are silicon, not C.
    bool TS(char *p) {
        bool original_value = *p; // Atomically read the original value
        *p = TRUE;                // Atomically set the value to TRUE
        return original_value;      // Return the original value
    }
    ```
    *   **Usage:** A memory location (`flag`) represents the lock, where `FALSE` means free and `TRUE` means held.
        *   A thread calls `TS(&flag)`.
        *   If `TS` returns `FALSE`, the lock was free. The thread has now acquired it, as `TS` set the flag to `TRUE`.
        *   If `TS` returns `TRUE`, the lock was already held by someone else. The thread did not acquire it.

*   **Compare-and-Swap Instruction:**
    *   This C-like description represents a single, atomic `compare_and_swap` instruction.
    ```c
    // This is a C description of a single machine language instruction.
    bool compare_and_swap(int *p, int old_val, int new_val) {
        if (*p == old_val) { // Atomically check if current value matches old_val
            *p = new_val;    // If so, atomically update to new_val
            return TRUE;     // Indicate success
        } else {
            return FALSE;    // Indicate failure (value was changed by someone else)
        }
    }
    ```    *   **Usage:** To acquire a lock, a thread calls `compare_and_swap(&flag, UNUSED, IN_USE)`.
        *   If the function returns `TRUE`, it means the flag was `UNUSED`, and the thread successfully changed it to `IN_USE`, acquiring the lock.
        *   If it returns `FALSE`, the flag was not `UNUSED`, meaning another thread held the lock.

***
#### 1.6.3 Implementing a Lock with Atomic Instructions

While atomic instructions like Test-and-Set are the fundamental building blocks, a complete lock requires more than just the raw instruction. A software layer is needed to define the lock and unlock operations.

Here is a simple implementation of `getlock` and `freelock` functions using the `TS` atomic instruction:

```c
// Assumes 'lock' is a type that can be passed to TS, e.g., char or int.
// 'TS' is a placeholder for the atomic test-and-set hardware instruction.

// Attempts to acquire the lock.
// Returns TRUE on success, FALSE on failure.
bool getlock(lock *lockp) {
    // TS returns the *original* value. 
    // If the original value was 0 (free), the call succeeds.
    if (TS(lockp) == 0) {
        return TRUE; // Lock acquired successfully
    } else {
        return FALSE; // Lock was already held
    }
}

// Releases the lock.
// This should only be called by the thread that holds the lock.
void freelock(lock *lockp) {
    // Set the lock's value back to 0 (free).
    // This operation itself does not need to be atomic, as only the
    // lock holder should ever call this function.
    *lockp = 0; 
}
```

**How it Works:**
*   **`getlock`:** A thread calls this function to acquire the lock. It uses the `TS` instruction, which atomically checks the lock's state and sets it to "held" (non-zero). If `TS` returns `0`, it means the lock was previously free, and the thread has successfully acquired it. If it returns non-zero, the lock was already held, and the acquisition fails.
*   **`freelock`:** The thread that holds the lock calls this function when it is done with the critical section. It simply resets the lock's memory location to `0` (free), allowing another thread to acquire it.

**Limitations and The Waiting Problem:**
This implementation is minimal. The `getlock` function simply returns `TRUE` or `FALSE`. It does not specify what the calling thread should do if it fails to get the lock (i.e., when `getlock` returns `FALSE`). The caller is responsible for the waiting strategy. For example, the caller might implement a spin lock:

```c
// Caller uses getlock to implement a spin lock
while (getlock(&my_lock) == FALSE) {
    // Spin: do nothing, just try again
}

// ... critical section ...

freelock(&my_lock);
```
This brings back the issues of spin-waiting, such as wasted CPU cycles. More advanced lock implementations build on this foundation by incorporating blocking mechanisms like condition variables to handle failed acquisition attempts more efficiently.

***
#### 1.6.4 Example: `pthreads`
The `pthread` library provides a mutex for locking. To protect the `counter = counter + 1` operation, the code is wrapped in lock and unlock calls.

```c
pthread_mutex_t lock;
pthread_mutex_init(&lock, NULL); // Initialize the lock

// ... in each thread's code ...

// Acquire the lock before entering the critical section
if (pthread_mutex_lock(&lock) == 0) {
    // ---- Critical Section Start ----
    counter = counter + 1; // This operation is now mutually exclusive
    // ---- Critical Section End ----
    
    // Release the lock
    pthread_mutex_unlock(&lock);
}
```

***
#### 1.6.5 Lock Enforcement
Locking mechanisms are effective only if they are used correctly and consistently. There are two primary enforcement models:
1.  **Mandatory Locking:** The system (e.g., the OS) makes it impossible to access the resource without first acquiring the lock. This is the most robust method.
2.  **Advisory/Cooperative Locking:** All programmers must agree to follow the protocol of acquiring the lock before accessing the resource. This model is prone to error, as a single non-compliant piece of code can violate mutual exclusion.

---
## 2.0 Asynchronous Completion

The **asynchronous completion problem** addresses how a process or thread should efficiently wait for an event that will be completed by another parallel activity (e.g., another thread, the OS, or a hardware device). The goal is to perform such waits without wasting CPU resources.

**Examples of Asynchronous Completions:**
*   Waiting for an I/O operation (e.g., disk read) to finish.
*   Waiting for a response to a network request.
*   Waiting for a specific amount of real time to pass.
*   Waiting for a lock to become available.

***
### 2.1 Waiting Strategies

When a thread cannot proceed because it is waiting for an event (like a lock being unavailable), it has several options.

***
#### 2.1.1 Spin Waiting (Busy-Waiting)
In spin waiting, a thread repeatedly checks for a condition in a tight loop until it becomes true. When used for locks, this is called **spin locking**.

*   **Process:**
    1.  Check if the event has occurred (e.g., if the lock is free).
    2.  If not, immediately check again.
    3.  Repeat until the event occurs.
*   **Advantages:**
    *   **Correctness:** Properly enforces access if locks are implemented correctly.
    *   **Simplicity:** Easy to program (often a simple `while` loop).
*   **Disadvantages:**
    *   **Wasteful:** Consumes CPU cycles performing no useful work. These cycles could be used by other threads.
    *   **Can Delay Resolution:** On a single-core system, if a thread is spinning, the thread that holds the resource it is waiting for cannot run to release it. The spinning thread actively prevents the condition it is waiting for from being met.
    *   **Bugs:** A bug could lead to an infinite spin-wait, hanging the thread.

***
#### 2.1.2 When Spinning is Appropriate
Spinning can be an efficient strategy under specific conditions:
1.  **Parallel Operation:** The awaited operation is proceeding in parallel on another core or hardware device.
2.  **Guaranteed Short Wait:** The event is known to occur very soon (e.g., within microseconds). The overhead of blocking and rescheduling the thread would be greater than the cost of spinning.
3.  **No Delay to Awaited Operation:** Spinning does not prevent the awaited event from happening (true in multi-core systems).
4.  **Rare Contention:** If contention for the resource is infrequent, the simplicity of spinning is advantageous, as the wait time will likely be zero.

***
#### 2.1.3 Yield and Spin
This is a hybrid approach where a thread spins for a limited number of attempts and then `yields` the CPU if the event has not occurred. Yielding tells the scheduler to run another thread.

*   **Process:**
    1.  Check for the event a few times.
    2.  If the event has not occurred, call `yield()`.
    3.  When the thread is rescheduled, repeat the process.
*   **Problems:**
    *   **Context Switch Overhead:** Yielding causes a context switch, which is expensive.
    *   **Wasted Cycles:** Still wastes cycles each time it is scheduled and spins before yielding again.
    *   **Poor Timing:** The event might occur right after the thread yields, causing it to wait a full scheduling cycle unnecessarily.
    *   **Unfairness:** With multiple waiters, there is no guarantee that the thread that has been waiting the longest will acquire the resource. A newly arrived thread might get lucky and acquire it first. This can lead to **starvation**, where a thread is perpetually denied access.

***
### 2.2 Completion Events and Condition Variables

A more efficient alternative to spinning is to block the waiting thread. This is managed using **completion events**, which are typically implemented with **condition variables**.

*   **Mechanism:**
    1.  When a thread finds it must wait (e.g., a lock is unavailable), it does not spin. Instead, it **blocks**.
    2.  Before blocking, it registers its interest in a specific event with the OS or thread library.
    3.  The OS moves the thread from the ready queue to a special waiting queue associated with that event. The thread consumes no CPU cycles while blocked.
    4.  When the event occurs (e.g., the lock is released), the responsible entity **posts** the event.
    5.  The OS is notified, finds the thread(s) in the waiting queue for that event, and moves one or more of them back to the ready queue.
    6.  Once scheduled, the awakened thread can retry its operation.

*   **Condition Variables:** A synchronization object associated with a specific event or resource. It has two main states: "event has not happened" and "event has happened" (posted). It also maintains a queue of threads waiting for the event.

***
### 2.3 Handling Multiple Waiters and Ensuring Fairness

When multiple threads are waiting for the same event, the system must manage them in an orderly and fair manner. This is achieved through dedicated waiting lists and explicit wake-up policies.

*   **Dedicated Waiting Lists:**
    Each condition variable maintains its own private list of threads that are waiting on it. This list is entirely separate from the scheduler's general-purpose ready queue. This separation is crucial for efficiency, as it allows the system to be precise. When a specific event is "posted" (signaled), only the threads on the corresponding waiting list are considered for wakeup, leaving all other threads undisturbed.

*   **Wake-up Policies:**
    When a signaler posts an event, it must choose a policy for waking up the threads on the waiting list. The two standard policies offer a trade-off between targeted efficiency and broad notification.

    1.  **Signal (e.g., `pthread_cond_signal`): Wake One Thread**
        *   **What it does:** This policy wakes up *at least one* waiting thread, typically the one that has been waiting the longest (FIFO).
        *   **When to use it:** Use `signal` when only one thread can make progress after the condition becomes true. This is common in producer-consumer scenarios (waking one consumer to take one item) or when managing an exclusive resource (waking one thread to try to acquire a lock). It is efficient because it avoids waking threads that would fail to proceed anyway.

    2.  **Broadcast (e.g., `pthread_cond_broadcast`): Wake All Threads**
        *   **What it does:** This policy wakes up *all* threads currently waiting on the condition variable.
        *   **When to use it:** Use `broadcast` when a change in state might allow multiple threads to make progress. For example, a writer thread might change a configuration flag that all reader threads need to see, or a resource might become available for shared (non-exclusive) access.
        *   **The "Thundering Herd" Problem:** A common pitfall is using `broadcast` when only one thread can actually proceed. This causes all waiting threads to wake up simultaneously, creating a "thundering herd" that stampedes to acquire the mutex. Only one will succeed; the rest will find the condition false again and go back to sleep. This contention is highly inefficient and wastes significant CPU time on pointless context switches.

*   **Ensuring Fairness and Preventing Starvation:**
    Fairness is a primary concern in concurrent systems. **Starvation** occurs when a thread is perpetually denied access to a resource while other threads make progress. Condition variables help prevent this:
    *   **FIFO Queues:** By managing the waiting list as a FIFO (First-In, First-Out) queue, the `signal` policy ensures that threads are served in the order they began waiting. This guarantees that no thread will wait forever, as long as the condition is signaled periodically.
    *   **Priority Systems:** In some specialized systems like Real-Time Operating Systems (RTOS), waiting lists might be ordered by thread priority instead of arrival time. While this helps meet critical deadlines, it introduces the risk of starving lower-priority threads if higher-priority threads are constantly waiting for the same resource.

***
### 2.4 The Sleep/Wakeup Race Condition and Its Solution

A subtle but severe race condition can occur when implementing blocking waits, leading to a thread sleeping forever. 

***
**The Race At Work:**
The race occurs in the small window between a thread checking a condition and subsequently blocking if the condition is false. The problem is illustrated by the following pseudocode implementation of `sleep` and `wakeup` functions.

```c
/* Code vulnerable to the sleep/wakeup race condition */

// Called by a thread that needs to wait for an event.
void sleep(eventp *e) {
    // Check if the event has already been posted.
    while (e->posted == FALSE) {
        // If not, add self to the event's waiting queue...
        add_to_queue(&e->queue, myproc); // This queue is a waiting list.
        
        // ...mark self as blocked...
        myproc->runstat |= BLOCKED;

        // ...and yield the CPU.
        yield();
    }
}

// Called by another thread to post an event and wake a waiter.
void wakeup(eventp *e) {
    struct proce *p;

    // Post the event.
    e->posted = TRUE;

    // Dequeue a waiting process, if any.
    p = get_from_queue(&e->queue);

    if (p) {
        // If someone was waiting, unblock them and reschedule.
        p->runstate &= ~BLOCKED;
        resched();
    } 
    /* if !p, nobody's waiting */
}
```

The race condition unfolds in this specific sequence of events:

1.  **Thread A** intends to wait. It calls `sleep()` and executes the `while (e->posted == FALSE)` check. The condition is `FALSE`, so it enters the loop.
2.  **Context Switch:** Before Thread A can execute `add_to_queue()`, a context switch occurs.
3.  **Thread B** runs. It completes its task, making the condition true, and calls `wakeup()`.
4.  The `wakeup()` function sets `e->posted = TRUE`. It then checks the waiting queue by calling `get_from_queue()`. Finding it empty (because Thread A isn't there yet), it concludes no one is waiting and simply returns. The "wakeup" signal is lost.
5.  **Context Switch:** Thread A runs again. Unaware that the condition is now true and the wakeup has already happened, it resumes inside the `while` loop and executes `add_to_queue()`, followed by setting its state to `BLOCKED` and calling `yield()`.

**The Effect:** Thread A is now blocked and waiting for a wakeup signal that will never come, causing it to sleep indefinitely.

**Solving the Problem: A Deeper Analysis**

The solution requires recognizing and correctly protecting a critical section within the `sleep()` and `wakeup()` logic.

1.  **Defining the Critical Section:** The critical section in the `sleep()` function is larger than it first appears. It must begin **before** the condition is checked and end only **after** the thread is successfully on the waiting list and has been marked as blocked. This entire sequence—checking the condition, adding to the queue, and blocking—must be atomic relative to the `wakeup()` operation.

2.  **The Mutual Exclusion Problem:** During this critical section, two things must be prevented:
    *   The `wakeup()` function for the event must not run.
    *   Other threads must not attempt to `sleep()` on the same event, which would cause concurrent modification of the waiting list.
    This is a classic mutual exclusion problem that requires a lock.

3.  **The Recursive Challenge:** The obvious solution is to protect the waiting list logic with a lock. However, a critical question arises: what *kind* of lock? If we use another high-level blocking lock (which itself uses a waiting list and condition variables), we create a recursive problem. A race condition could occur on the waiting list for our *first* lock, which would require a *second* lock, and so on, leading to infinite regress.

4.  **The Real Solution: A Low-Level, Non-Blocking Lock:** The recursion is broken by using a low-level lock that does not itself block. The critical section in the sleep/wakeup logic must be protected by a **spinlock**, which is built directly from atomic hardware instructions (like `test-and-set` or `compare-and-swap`). This spinlock provides the required mutual exclusion for the brief period needed to check the condition and modify the waiting list, but it does so without creating another blocking wait, thus avoiding the sleep/wakeup race condition entirely.

---
## 3.0 Conclusion

There are two fundamental classes of synchronization problems: mutual exclusion and asynchronous completion.

*   **Mutual exclusion** addresses the need to protect shared resources within critical sections, ensuring that only one thread operates at a time. The primary software abstraction for this is the **lock** (or mutex).

*   **Asynchronous completion** addresses the need for threads to wait efficiently for events. This is handled through two main strategies: **spinning** for very short waits and **completion events** (implemented with condition variables) for efficient, non-wasteful blocking.

Critically, the solutions to both of these high-level problems rely on a common, low-level foundation: **atomic hardware instructions**. These uninterruptible operations are the primitives used to build reliable software locks and to safely implement the internal mechanics of condition variables, preventing race conditions within the synchronization tools themselves.
