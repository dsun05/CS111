# CS 111: Memory Management

### 1. Introduction to Memory Management
Memory management is a critical function of an operating system that involves managing the computer's primary memory, or Random Access Memory (RAM). According to the von Neumann architecture, a computer's instructions and data must be in RAM to be processed by the CPU.

The central challenge is that RAM is a **finite and expensive resource**, while modern systems run hundreds of processes simultaneously, all of which require memory. The operating system must therefore have an effective strategy for sharing the limited RAM among all competing processes.

#### 1.1. Goals of Memory Management
The primary goals of a memory management system are:
1.  **Transparency**: Each process should operate as if it has its own private address space. It should be completely unaware that it is sharing memory with other processes.
2.  **Efficiency**: The system should achieve high memory utilization, ensuring that the limited RAM is used effectively and not wasted. Furthermore, the overhead costs of memory management tasks (like allocation and deallocation) should be very low.
3.  **Protection and Isolation**: The system must prevent processes from accessing or corrupting each other's memory or the operating system's memory. Private data must remain private and secure. Hardware support is crucial for enforcing these boundaries reliably.

---
### 2. Physical vs. Virtual Addresses
It is important to distinguish between two types of addresses used in computing.

* **Physical Address**: A physical address corresponds to an actual, physical location on a RAM chip. Early computers used only physical addresses, meaning a program's reference to address `1,000,000` was a direct reference to the millionth memory word on a hardware chip.
* **Virtual Address**: A virtual address is an address used by a process that is independent of the physical hardware. A process might refer to virtual address `1,000,000`, but the actual data may be stored at a completely different physical address or not even be in RAM at all (e.g., it could be on a disk).

Modern systems use virtual addresses because they provide enormous flexibility. However, this requires a **translation** mechanism, typically with hardware support, to convert virtual addresses into physical addresses before the memory can be accessed. This lecture focuses primarily on managing physical memory, with an introduction to virtual addresses at the end.

---

### 3. Key Challenges in Memory Management
An operating system's memory manager must contend with several fundamental challenges:
* **Unpredictable Memory Needs**: Most processes cannot perfectly predict how much memory they will require ahead of time. This means they might request more than they need to be safe, or they might need their memory allocation to grow during execution.
* **Data Persistence**: A process expects that when it stores data at a memory location, that data will remain unchanged until the process itself changes it. The OS must guarantee that another process cannot access or modify it.
* **Overallocation**: The total true memory requirements of all running processes, plus the OS itself, will almost certainly be greater than the amount of physical RAM available in the system. The memory manager must employ strategies to handle this.
* **Fast Process Switching**: Context switching between processes must be very fast. If switching required copying large amounts of a process's memory, the performance delays would be unacceptably high.
* **Low Overhead**: The algorithms used to manage memory (e.g., to allocate, deallocate, or track partitions) must be computationally inexpensive. High overhead would steal CPU cycles from user processes and slow down the entire system.

---

### 4. Fixed Partition Allocation
Fixed partition allocation is one of the earliest and simplest memory management strategies.

#### 4.1. How It Works
The total physical memory is divided into a number of **fixed-size partitions**. These partitions can be all one size or a few different sizes (e.g., some 4MB partitions and some 8MB partitions).

When a process needs memory, the operating system finds an unused partition that is large enough to satisfy the request and allocates the *entire partition* to that process. Each partition is owned by one and only one process at a time.

This method is:
* **Simple to implement**: Allocation and deallocation are cheap, often managed with a simple **bitmap** where each bit represents the status (free or used) of a partition.
* **Well-suited for predictable systems**: It works well in special-purpose or embedded systems where the memory needs of all processes are known in advance.

#### 4.2. Problems with Fixed Partitions
This strategy suffers from several major drawbacks:
* **Inflexible**: The number and size of partitions are fixed. A process cannot get more memory than its allocated partition size while it is running.
* **Limited Concurrency**: The total memory required by all running processes cannot exceed the available physical RAM.
* **Internal Fragmentation**: It is highly susceptible to a form of memory waste known as internal fragmentation.

#### 4.3. Internal Fragmentation
**Internal fragmentation** is wasted memory that exists *inside* an allocated block or partition. It occurs because a process is given a partition that is larger than its actual memory requirement. The unused space within that partition cannot be allocated to any other process and is therefore wasted.

On average, if the requested sizes are random, about 50% of the space in each allocated block can be wasted.

**Example of Internal Fragmentation**
Consider a system with three available partitions and three incoming processes.

| Process Request | Available Partitions |
| :-------------- | :------------------- |
| Process A: 6 MB | Partition 1: 8 MB    |
| Process B: 3 MB | Partition 2: 4 MB    |
| Process C: 2 MB | Partition 3: 4 MB    |

The allocation would proceed as follows:
* Process A (needs 6 MB) is allocated the 8 MB partition. **Waste: 2 MB**.
* Process B (needs 3 MB) is allocated a 4 MB partition. **Waste: 1 MB**.
* Process C (needs 2 MB) is allocated the other 4 MB partition. **Waste: 2 MB**.

In this scenario, a total of 5 MB of memory is wasted out of 16 MB available (31% waste). If a new process requests a 3 MB partition, the system cannot satisfy the request, even though 5 MB of memory is technically unused.

---
### 5. Dynamic Partition Allocation
Dynamic partition allocation addresses the problem of internal fragmentation by allocating partitions that are exactly the size requested by a process.

#### 5.1. How It Works
Instead of pre-defining partitions, the operating system treats memory as one large block, or **heap**. When a process requests a partition of a specific size, the OS finds a contiguous block of free memory of that exact size and allocates it.

This eliminates internal fragmentation because processes are given no more memory than they request. However, it introduces other significant problems.

#### 5.2. Problems with Dynamic Partitions
1.  **Not Relocatable**: Once a partition is allocated at a specific physical address range, it cannot be easily moved. The program's internal pointers contain absolute physical addresses, which would become invalid if the partition were moved.
2.  **Not Easily Expandable**: If a process needs to increase its memory allocation, it can only do so if there is free, contiguous memory immediately adjacent to its current partition. If the adjacent memory is already allocated to another process, the request cannot be met, even if there is enough free memory elsewhere.
3.  **External Fragmentation**: This strategy is highly prone to external fragmentation.

#### 5.3. External Fragmentation
**External fragmentation** is wasted memory that exists *outside* of any allocated partition. It occurs when the free memory is broken up into many small, non-contiguous blocks ("holes").

Over time, as processes are allocated and freed, the available memory becomes checkerboarded with small, unusable fragments. The total amount of free memory might be substantial, but if no single free block is large enough to satisfy a new request, that request will fail.

#### 5.4. Managing the Free List
To manage variable-sized partitions, the OS maintains a data structure, typically a **linked list**, called the **free list**. This list keeps track of all the free blocks of memory. Each node in the list contains information about a free block, such as its starting address and its size.

When a process requests memory, the OS searches the free list for a block that is large enough. If a suitable block is found, the OS will **carve** out the exact amount needed for the process and return the leftover portion to the free list as a new, smaller free block.

---
### 6. Combating External Fragmentation
There are two main approaches to combat external fragmentation: using smarter allocation algorithms and recombining free blocks.

#### 6.1. Allocation Algorithms
When searching the free list for a block to satisfy a request, different algorithms can be used. Each represents a trade-off between search time and fragmentation.

* **Best Fit**: Scans the *entire* free list and chooses the smallest available block that is large enough to satisfy the request.
    * **Advantage**: Might find a perfect fit, leaving no fragment.
    * **Disadvantage**: Very slow, as it must search the whole list every time. It tends to create many tiny, unusable fragments very quickly.
* **Worst Fit**: Scans the *entire* free list and chooses the largest available block.
    * **Advantage**: The leftover fragment is as large as possible, making it more likely to be useful for future requests.
    * **Disadvantage**: Also very slow. Over time, it can destroy large blocks that might be needed later.
* **First Fit**: Scans the free list from the beginning and chooses the *first* block it finds that is large enough.
    * **Advantage**: Much faster search time than Best or Worst Fit.
    * **Disadvantage**: Tends to create many small fragments at the beginning of the list, which slows down future searches. Performance degrades to that of Best Fit over time.
* **Next Fit**: Similar to First Fit, but it starts each search from the location where the last search finished, rather than from the beginning of the list. It uses a "guess pointer" that roams through the list.
    * **Advantage**: Spreads the fragmentation evenly throughout memory instead of concentrating it at the beginning. Search times are generally short.

In practice, simulations show that **Worst Fit and Next Fit generally perform better than Best Fit and First Fit** in terms of both speed and memory utilization.

#### 6.2. Coalescing
**Coalescing** is the process of merging adjacent free blocks into a single, larger free block. Whenever a partition is freed, the operating system checks its neighbors. If the block before or after it is also free, they are combined. This actively works against external fragmentation by creating larger, more useful blocks of free memory. To make this process efficient, the free list is typically kept sorted by memory address.

---
### 7. Advanced Strategies

#### 7.1. Buffer Pools
Analysis of real systems shows that memory requests are often not random. Certain request sizes are extremely common (e.g., sizes corresponding to disk blocks or network packets). A **buffer pool** is a dedicated region of memory that is pre-divided into a pool of fixed-size buffers of a popular size.

When a request comes in for that exact size, it is satisfied from the buffer pool instead of the general-purpose dynamic allocator.
* **Benefits**: Extremely fast and efficient. It avoids the overhead of searching, carving, and coalescing, and it eliminates external fragmentation for these common requests.
* **Usage**: To avoid internal fragmentation, a buffer pool only satisfies requests that are a perfect size match. The size of the pool itself can be adjusted dynamically based on demand.

#### 7.2. Garbage Collection
**Memory leaks** occur when a program allocates memory but fails to release it after it's no longer needed. In long-running applications, this can lead to a gradual loss of available memory.

**Garbage collection** is an automated process that reclaims this leaked memory. The garbage collector periodically scans memory to identify which objects are still reachable (i.e., have a valid pointer to them). Any memory that is found to be unreachable is considered "garbage" and is added back to the free list. While very effective in managed languages (like Java) or for specific OS resources, implementing a general-purpose garbage collector for languages like C is extremely difficult because it's hard to distinguish a pointer from other integer data.

#### 7.3. Memory Compaction
Compaction is a direct solution to external fragmentation. The operating system temporarily stops processes, moves all allocated partitions together into one end of memory, and consolidates all the small, fragmented holes into one large block of free memory at the other end.

While this is very effective, it is also very expensive. It requires the ability to **relocate** partitions, which is not possible if programs are using absolute physical addresses.

---
### 8. Relocation and the Introduction to Virtual Memory

#### 8.1. The Relocation Problem
As discussed, moving a program in physical memory is difficult because all of its internal pointers (to code locations, data structures, stack frames) contain physical addresses. If the program is moved, these pointers become invalid and the program will crash. The solution is to make programs **location-independent**.

#### 8.2. Segment Relocation with Base and Limit Registers
An early form of virtual memory uses hardware **segment registers** to solve the relocation problem. A process's address space is divided into logical **segments** (e.g., a code segment, a data segment, a stack segment).

For each segment, the hardware provides two special CPU registers:
1.  **Base Register**: Stores the starting *physical address* of the segment in RAM.
2.  **Limit Register**: Stores the *size* (or length) of the segment.

When a process generates a virtual address, the hardware automatically performs the following steps:
1.  **Translation**: It adds the virtual address to the value in the appropriate base register to get the final physical address.
    `Physical Address = Virtual Address + Base Register`
2.  **Protection**: It checks if the virtual address is less than the value in the limit register. If it is not (`Virtual Address >= Limit`), the access is out of bounds, and the hardware generates a **segmentation fault** (a trap to the OS), which typically terminates the process.

With this mechanism, **relocating a segment is easy**. The OS can copy the segment to a new location in physical memory and simply update the value in its corresponding base register. The process itself is unaware of the change, as its virtual addresses remain the same. This allows for effective memory compaction.

#### 8.3. Conclusion and Next Steps
Base-and-limit registers solve the relocation problem, enabling compaction to fight external fragmentation. However, this approach still requires that each segment be stored in a contiguous block of physical memory, so external fragmentation remains a problem. Furthermore, it does not allow the total memory used by all processes to exceed the physical RAM available. These final challenges are solved by more advanced memory management techniques, such as paging, which will be covered in the next lecture.
