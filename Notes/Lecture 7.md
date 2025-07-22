# Lecture 7: Threads, IPC, and Synchronization

## 1.0 Threads

A thread is a fundamental unit of execution and scheduling within an operating system. While a process is also an interpreter of instructions, threads offer a more lightweight alternative for managing parallel activities within a single program.

### 1.1 Why Processes Are Not Always Sufficient

Processes are powerful but come with significant overhead, making them unsuitable for all scenarios.

*   **High Cost:**
    *   **Creation:** Creating a process is expensive in terms of performance. It involves setting up a new process control block (PCB), creating a page table, allocating page frames for that table, and adding the process to the scheduler's queue.
    *   **Dispatching (Context Switching):** Switching between processes is also costly. It requires switching to a different address space, which invalidates hardware caches (like the TLB), leading to performance degradation.

*   **Strict Separation:**
    *   Processes are designed to be isolated from one another. They do not share the same address space or other resources like open files.
    *   This strong separation is excellent for security and stability but is inconvenient for cooperative tasks that need to share data or resources extensively, such as a web server handling multiple client requests. In such cases, the elements of the application trust each other and need to work on shared data, making the process model's isolation a hindrance.

### 1.2 What is a Thread?

A thread is an interpreter that runs instructions but is only a unit of execution and scheduling, not resource ownership.

*   **Thread-Specific Resources:** Each thread has its own:
    *   **Stack:** Necessary for executing code, managing function calls, and storing local variables.
    *   **Program Counter (PC):** Keeps track of the next instruction to execute for that thread.
    *   **Set of Registers:** Stores the current working values for the thread's computations.

*   **Shared Resources:** All other resources are shared among threads belonging to the same process.
    *   **Address Space:** All threads within a process share the same memory space. This is a critical feature, as it means switching between threads does not require an expensive address space switch.
    *   **Open Files:** If one thread opens a file, all other threads in that process can access it.
    *   **Data Space:** Threads share the same global variables and heap memory.

Because they share so many resources, threads are much cheaper to create, destroy, and switch between than processes.

### 1.3 User-Level vs. Kernel-Level Threads

There are two primary models for implementing threads:

*   **User-Level Threads:**
    *   The operating system kernel is unaware of the existence of these threads.
    *   They are managed entirely by a library within the user process (e.g., the `pthread` library).
    *   This library handles thread creation, scheduling, and context switching.
    *   **Scheduling:** Since the kernel sees only a single process, if one user-level thread makes a blocking system call (e.g., for I/O), the entire process blocks, even if other threads are ready to run. The library has its own non-preemptive scheduler, often relying on threads to voluntarily yield the CPU.
    *   **Advantages:** Very fast to create and switch because no kernel intervention is required.
    *   **Disadvantages:** Cannot take advantage of multi-core processors, as the kernel can only assign the single process to one core at a time. A blocking call stalls all threads.

*   **Kernel-Level Threads:**
    *   The kernel recognizes and manages threads directly. It knows that a process can contain multiple threads of control.
    *   The OS kernel handles thread creation, destruction, and scheduling.
    *   **Scheduling:** The kernel schedules threads, not just processes. It can preempt threads and make more informed scheduling decisions.
    *   **Advantages:** Allows multiple threads from the same process to run in parallel on different CPU cores, leading to true parallelism and significant performance gains. If one thread blocks, the kernel can schedule another thread from the same process to run. Modern systems like Linux provide kernel threads.
    *   **Disadvantages:** Creating and managing kernel threads is slower than user-level threads because it requires system calls and kernel intervention.

### 1.4 Managing Thread Stacks

Because each thread has its own stack, but all threads share a single address space, stack management is a crucial issue. The simple model of a stack and a heap growing towards each other does not work with multiple stacks.

*   **Solution:** When a thread is created, a specific region of the process's address space is allocated for its stack.
*   **Maximum Stack Size:** A maximum size for the stack must be specified at creation time. This is necessary to ensure that one thread's stack does not grow so large that it overwrites memory belonging to another thread's stack or other data.
*   **Stack Overflow:** If a thread's stack usage exceeds its allocated maximum size, it results in a failure (a stack overflow error), and the thread is typically terminated.
*   **Reclamation:** When a thread exits, the memory space allocated for its stack must be reclaimed so it can be used for other purposes.

### 1.5 When to Use Threads vs. Processes

| Use Processes When...                                 | Use Threads When...                                     |
| ----------------------------------------------------- | ------------------------------------------------------- |
| Running multiple, distinct, unrelated programs.        | Performing parallel activities within a single program. |
| Creation and destruction of interpreters are rare events. | Creation and destruction of interpreters are frequent.  |
| Agents need distinct privileges and access rights.    | All agents can run with the same privileges.            |
| Interactions and resource sharing are limited.        | Agents need to share resources (like data structures) extensively. |
| You need to prevent interference between interpreters.  | Agents trust each other and protection is not a primary concern. |
| You want to firewall one part of the application from the failure of another. A process crash doesn't take down other processes. | A failure in one thread is acceptable to bring down the entire process. |

## 2.0 Inter-Process Communication (IPC)

**Inter-Process Communication (IPC)** refers to the set of mechanisms provided by an operating system that allow distinct processes to communicate and synchronize their actions. By default, processes are isolated, so IPC mechanisms are necessary to facilitate cooperation.

### 2.1 Goals and Implementation of IPC

IPC mechanisms are designed with several, sometimes contradictory, goals in mind:

*   **Simplicity & Convenience:** Easy for programmers to use.
*   **Generality:** Supports many different communication patterns.
*   **Efficiency:** Low performance overhead.
*   **Robustness & Reliability:** Tolerant of errors and ensures data integrity.

Because these goals conflict, operating systems typically provide multiple IPC mechanisms, each suited for different needs. IPC is provided through system calls, requiring cooperative action from both the sending and receiving processes.

### 2.2 Fundamental Data Transfer Methods
At the lowest level, moving bits from Process A to Process B on the same machine happens in one of two ways:

*   **Data Copying:** This is the most common method.
    *   Process A copies data from its user space into a buffer inside the kernel's address space (1st copy).
    *   The kernel then copies the data from its internal buffer into the user space of Process B (2nd copy).
    This "double copy" is safe and simple for the OS to manage but introduces performance overhead. Pipelines, Sockets, and Messages all use this method.

*   **Memory Mapping (Sharing):** This is a much faster method used by Shared Memory IPC.
    *   The OS adjusts the page tables of both Process A and Process B so that a specific range of virtual addresses in each process points to the *exact same physical page frames* in RAM.
    *   After this initial setup, there are **zero copies**. When Process A writes to this memory region, Process B can immediately read the new data because they are looking at the same physical hardware memory cells. The OS is not involved in the subsequent data transfers.

### 2.3 Synchronous vs. Asynchronous IPC

*   **Synchronous IPC:** In this model, the communication calls are **blocking**.
    *   **Sender:** A `send` operation blocks the sending process until the message has been delivered or at least received by the OS and is on its way.
    *   **Receiver:** A `receive` operation blocks the receiving process until the data is available.
    *   **Advantage:** Simple to program and reason about because the state is clear at each step.

*   **Asynchronous IPC:** In this model, the communication calls are **non-blocking**.
    *   **Sender:** A `send` operation returns control to the process as soon as the OS has accepted the data for transmission. The sender gets no immediate confirmation that the data was delivered successfully. An auxiliary mechanism is needed to learn of errors (e.g., the receiver crashed).
    *   **Receiver:** A `receive` operation checks if data is available. If it is, the data is retrieved. If not, the call returns immediately with a status indicating no data, allowing the process to do other work instead of blocking.
    *   **Advantage:** More efficient in some circumstances, as it allows processes to overlap computation with communication and avoid unnecessary blocking.

### 2.4 Typical IPC Operations

While specific system calls vary between operating systems, most IPC frameworks provide a common set of operations to manage communication:

*   **Create/Destroy Channel:** An explicit step to establish a communication channel before use and to tear it down afterward to free up system resources.
*   **Send/Write/Put Data:** Operations to place data into the established channel from the sending process.
*   **Receive/Read/Get Data:** Operations to extract data from the channel on the receiving end.
*   **Query Channel Status:** The ability to ask the OS about the state of the channel, such as "How much data is currently buffered?" This is useful for flow control and decision-making.
*   **Connection Establishment and Query:** Operations that help processes find each other and set up a connection. This can involve querying for communication partners, checking if they are ready to communicate, and controlling the connection endpoints.

### 2.5 Streams vs. Messages

IPC can be categorized by how it structures data:

*   **Streams:**
    *   Communication happens as a continuous stream of bytes, like in a Unix pipe (`ls | grep`).
    *   The sending process can write any number of bytes at a time, and the receiving process can read any number of bytes. There are no inherent message boundaries.
    *   If the application needs to define records or messages within the stream, it must embed its own delimiters. The IPC mechanism itself treats these delimiters as just more bytes.
    *   **Known by:** The application, not the IPC mechanism.

*   **Messages (Datagrams):**
    *   Communication is done in discrete packets or messages.
    *   The sender creates an entire message and then sends it in a single operation. The receiver gets the entire message in a single operation.
    *   The operating system understands the message boundaries. Delivery is typically all-or-nothing; the receiver either gets the full message or nothing.
    *   **Known by:** The IPC mechanism.

### 2.6 Flow Control

**Flow control** is the process of managing the rate of data transmission to prevent a fast sender from overwhelming a slow receiver.

*   **The Problem:** If a sender produces data faster than the receiver can consume it, the data must be stored somewhere in the interim.
*   **Buffering:** The OS typically uses a buffer (a queue in kernel memory) to hold data that has been sent but not yet received.
*   **Buffer Overflow:** This buffer has a finite size. If it fills up, the OS must take action.
    *   **Block the Sender:** The most common solution. The OS blocks the sending process until the receiver consumes some data, freeing up buffer space.
    *   **Refuse Communication:** In an asynchronous model, the OS might return an error to the sender's `send` request, indicating that the buffer is full. The sender must then decide what to do (e.g., wait and retry, or do something else).

### 2.7 IPC Examples

#### 1. Pipelines

Pipelines are a simple, stream-based IPC mechanism for connecting the output of one program to the input of another.

*   **Mechanism:** The OS creates an in-memory buffer. The sender writes bytes to the buffer, and the receiver reads bytes from it. No intermediate files are created.
*   **Use Case:** `ls | grep "foo" | sort`
*   **Characteristics:**
    *   Simple byte stream.
    *   Implicit trust: All processes in the pipeline run under the same user with the same privileges.
    *   Limited functionality but very effective for its purpose.

#### 2. Sockets

Sockets are a highly general and powerful IPC mechanism that can be used for communication between processes on the same machine or across a network.

*   **Mechanism:** Sockets work by establishing a connection between two endpoints, each identified by an address (IP address) and a port number.
*   **Setup:** Involves calls like `connect`, `listen`, and `accept` to establish the communication channel.
*   **Characteristics:**
    *   Extremely general: Can be configured for reliable, connection-oriented streams (like TCP) or unreliable, best-effort messages (like UDP).
    *   Complex: Offers many options for flow control, error handling, retransmissions, and security, which can make them difficult to configure correctly.
    *   Can work across a network.

#### 3. Shared Memory

Shared memory is the fastest possible IPC mechanism.

*   **Mechanism:** The OS maps the same physical page frames of RAM into the virtual address spaces of two or more cooperating processes.
*   **How it Works:**
    1.  Processes request a shared memory segment from the OS.
    2.  The OS creates the segment and adjusts the page tables of the participating processes so that a range of their virtual addresses points to the same physical memory pages.
*   **Characteristics:**
    *   **Extremely Fast:** Once set up, processes read and write to the shared memory directly at memory speeds. The OS kernel is not involved in the data transfer at all.
    *   **Synchronization is Crucial:** Because the OS is not mediating access, the processes themselves are entirely responsible for synchronizing their access to the shared memory to avoid corruption. This is a significant challenge.
    *   **Local Only:** Only works for processes on the same physical machine.

## 3.0 Synchronization

**Synchronization** is the task of coordinating the execution of parallel processes or threads to ensure they happen in the correct order, especially when they access shared resources. While parallelism is essential for modern performance, it introduces significant challenges.

### 3.1 The Problem of Parallelism

When multiple streams of instructions run independently, the outcome is predictable. However, when they cooperate and share resources, the result of a computation can depend on the specific, non-deterministic order in which their instructions are interleaved.

*   **Race Condition:** A situation where the outcome of a computation depends on the unpredictable timing or ordering of events, such as the interleaving of instructions from multiple threads. Most race conditions are harmless, but some can corrupt data and cause incorrect behavior.

*   **Non-Deterministic Execution:** The behavior of parallel programs is inherently less predictable due to factors like:
    *   Scheduling decisions (preemption).
    *   Interrupts.
    *   I/O delays.
    *   Queuing and network delays.

### 3.2 The Critical Section Problem

A **critical section** is a piece of code that accesses and modifies a shared resource (e.g., a data structure, a file, a global variable). The **critical section problem** arises when multiple threads or processes execute the same critical section concurrently, leading to a race condition.

To prevent this, we need to enforce **mutual exclusion**, which ensures that only one thread can be executing in the critical section at any given time.

#### Example 1: Updating a File

Two processes interact with an `inventory` file. Process 1 updates it by removing the old one and creating a new one. Process 2 reads it.

| Step | Process 1                     | Process 2                  | State of `inventory` file                               |
| :--: | ----------------------------- | -------------------------- | ------------------------------------------------------- |
|  1   | `remove("inventory");`         |                            | File does not exist.                                    |
|  2   | `fd = create("inventory");`    |                            | File exists, but is empty.                              |
|  3   | *(context switch)*            | `fd = open("inventory",READ);` | Process 2 successfully opens the empty file.            |
|  4   |                               | `count = read(...);`       | `count` becomes 0 because the file is empty.            |
|  5   | `write(fd,newdata,length);`   |                            | File is now filled with new data.                       |
|  6   | `close(fd);`                  |                            |                                                         |

**Result:** Process 2 incorrectly concludes the inventory is empty. This result could not happen if the processes ran sequentially in either order.

#### Example 2: Multithreaded Banking Code

Two threads operate on a shared `balance` variable, initially $100. Thread 1 deposits $50, and Thread 2 withdraws $25. The correct final balance should be $125.

Each high-level operation (e.g., `balance = balance + 50`) is not atomic; it consists of multiple machine instructions: load, add, store.

| Step | Thread 1 (Deposit $50)     | Thread 2 (Withdraw $25)    | `balance` | T1's R1 | T2's R1 |
| :--: | -------------------------- | -------------------------- | :-------: | :-----: | :-----: |
|  1   | `load r1, balance`         |                            |    100    |   100   |   --    |
|  2   | `add r1, 50`               |                            |    100    |   150   |   --    |
|  3   | *(context switch)*         | `load r1, balance`         |    100    |  (150)  |   100   |
|  4   |                            | `sub r1, 25`               |    100    |  (150)  |   75    |
|  5   |                            | `store r1, balance`        |    75     |  (150)  |   75    |
|  6   | *(context switch)*         |                            |    75     |   150   |  (75)   |
|  7   | `store r1, balance`        |                            |    150    |   150   |  (75)   |

**Result:** The final balance is $150. The $25 withdrawal was lost because Thread 1's `store` operation overwrote the result of Thread 2's `store` operation.

### 3.3 Solutions to the Critical Section Problem

#### 1. Disabling Interrupts (A Flawed Approach)

One could temporarily disable interrupts before entering a critical section and re-enable them upon exit. This would prevent the scheduler (which relies on timer interrupts) from preempting the thread mid-operation.

*   **Dangers and Downsides:**
    *   **Privileged Instruction:** User-level code cannot disable interrupts; only the OS kernel can.
    *   **Doesn't Work on Multi-Core Systems:** Disabling interrupts on one core does not stop another thread from running on a different core and accessing the same shared resource simultaneously.
    *   **Delays Important Events:** Disabling interrupts can cause the system to miss critical events, like I/O completions or power failure warnings.
    *   **Dangerous:** A bug could leave interrupts permanently disabled, crashing the system.
    *   **Conclusion:** This is not a general or safe solution for synchronization.

#### 2. Atomic Instructions and Locks

A better approach relies on hardware support in the form of **atomic instructions**. These are machine instructions that are guaranteed to execute to completion without interruption.

*   **Examples:** `test-and-set`, `compare-and-swap`, `increment`.
*   **Limitation:** They typically operate on a single, small piece of data (e.g., one word).
*   **Application:** While not sufficient to protect a large critical section directly, these atomic instructions can be used as building blocks to implement a higher-level synchronization mechanism called a **lock**. A thread must acquire the lock before entering a critical section and release it upon leaving. This will be covered in more detail in the next lecture.

## 4.0 Conclusion

*   Processes are a fundamental but expensive model for computation.
*   Threads offer a cheaper, more lightweight alternative for parallel tasks within a single program.
*   Threads communicate via shared memory, while processes require explicit IPC mechanisms like pipelines, sockets, or shared memory segments.
*   Both threads and processes enable parallelism, which is vital for performance in modern systems.
*   However, parallelism introduces complex synchronization problems (race conditions and non-determinism) that corrupt data if not managed correctly.
*   Solving these problems requires mechanisms to ensure mutual exclusion for critical sections of code.
