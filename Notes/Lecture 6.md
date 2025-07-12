# CS 111: Memory Management – Swapping, Paging, and Virtual Memory

## 1. Recap of Previous Memory Management Issues

In previous discussions on operating system memory management, basic strategies like fixed and dynamic partitions were introduced. These methods suffered from several key drawbacks:

  * **Fragmentation**: Both internal and external fragmentation led to inefficient use of memory. While variable-sized partitions eliminated internal fragmentation, they did so at the cost of worsening external fragmentation.
  * **Relocation**: Simple schemes made it difficult to relocate processes in memory once they had been placed.
  * **Contiguity Requirement**: A critical limitation of early approaches, including segmentation, is that each process segment (e.g., code, data, stack) must occupy a single, contiguous block of physical memory. While segmentation registers allow these contiguous blocks to be moved, coalescing free space is still hampered because a sufficiently large *single* block must be found. This requirement fundamentally limits the number of processes that can be run concurrently.

To overcome these limitations, more advanced techniques like swapping, paging, and virtual memory were developed.

-----

## 2. Swapping

Swapping was an early strategy used to address the problem of total memory requirements of all processes exceeding the available physical RAM.

***
### 2.1 Definition and Purpose

The core idea of swapping is to use secondary storage (like a hard disk or flash drive) as an overflow area for RAM. If there isn't enough RAM for all active processes, some processes can have their memory contents temporarily moved to the disk.

***
### 2.2 How Swapping Works

The process is straightforward:

1.  When a process is not running (e.g., it is blocked or its time slice has expired), the operating system can copy all of its memory segments (code, data, and stack) from RAM to a designated swap space on the disk. This frees up RAM for other processes.
2.  The swapped-out process cannot run while its state is on the disk.
3.  To run the process again, the OS must copy its memory segments back from the disk into RAM.
4.  With relocation hardware like segmentation registers, the process does not need to be loaded back into the exact same physical memory locations it previously occupied. The OS can place it wherever there is a large enough contiguous free block and simply update the process's base registers.

***
### 2.3 Downsides of Simple Swapping

While functional, this simple swapping approach has significant drawbacks:

  * **High Context-Switch Cost**: Swapping an entire process is a very slow, heavyweight operation involving large amounts of disk I/O. A context switch could involve writing one process's entire memory footprint to disk and reading another's in, causing extreme delays.
  * **Process Size Limitation**: Swapping does not solve the problem of a single process requiring more memory than is physically available in the system. Each process is still limited to a memory size that can fit entirely within RAM.

Because of these issues, simple swapping was considered a stopgap measure, and a more granular approach was needed.

-----

## 3. Paging

Paging is a memory management scheme that eliminates the requirement for contiguous physical memory, which is the primary source of external fragmentation and a major limitation of earlier methods.

***
### 3.1 The Core Concept of Paging

The paging approach divides memory into small, fixed-size chunks:

  * **Page Frame**: A fixed-size block of physical memory (RAM). Common sizes are 4KB or 16KB.
  * **Page**: A corresponding fixed-size block of a process's virtual address space. The page size is always equal to the page frame size.

The fundamental principle of paging is that **any virtual page from any process can be stored in any physical page frame**. This allows a process's memory to be scattered non-contiguously throughout physical RAM, as shown below.

#### Paged Address Translation Example

*A process's virtual segments are broken into pages, which are then placed into available page frames anywhere in physical memory. They do not need to be in order or contiguous.*

***
### 3.2 Paging and Fragmentation

Paging has a dramatic effect on memory fragmentation:

  * **External Fragmentation**: This is completely eliminated. Since memory is managed in fixed-size page frames, there are no oddly sized holes left between allocations. Every free frame is usable by any page.
  * **Internal Fragmentation**: This form of fragmentation reappears but is significantly reduced. A process segment is made up of a number of pages. All pages except for the very last one will be full. Only the final page of a segment might be partially filled, leading to some wasted space within that last page frame.
      * On average, the amount of wasted space per segment is half a page.
      * For a typical process with three segments (code, data, stack), this results in an average total waste of only 1.5 pages (e.g., 6KB with 4KB pages), which is a trivial amount in modern systems with gigabytes of RAM.

***
### 3.3 Paged Address Translation and the Memory Management Unit (MMU)

Since a process's pages are scattered, the system needs a mechanism to translate the logical (virtual) addresses generated by the process into the correct physical addresses in RAM. This translation must occur for every single memory reference and must be extremely fast.

  * **Memory Management Unit (MMU)**: A dedicated hardware component that performs this rapid virtual-to-physical address translation. Modern MMUs are integrated directly into the CPU chip to maximize speed.

#### The Translation Process

The MMU translates addresses as follows:

1.  A **virtual address** is divided by the hardware into two parts: a **virtual page number** (the higher-order bits) and an **offset** within that page (the lower-order bits).
2.  The MMU uses the virtual page number as an index into a data structure called the **page table**.
3.  The page table entry contains the **physical page frame number** where the corresponding page is located in RAM. It also contains bits like a **valid bit** to indicate if the mapping is legal.
4.  The MMU takes this physical page frame number and combines it with the original, unchanged offset to form the final **physical address**, which is then sent to the memory bus.

*The MMU hardware translates a virtual page number to a physical page frame number via the page table, while the offset remains the same.*

#### Page Tables

  * **The Size Problem**: In a 64-bit system with 4KB pages, the virtual address space is enormous, leading to potentially massive page tables (e.g., $2^{24}$ or 16 million entries for a 64GB memory). Storing these in fast on-chip registers is impossible due to their size.
  * **Solution 1: Store in RAM**: To solve the size issue, page tables for each process are stored in main memory (RAM).
  * **The Performance Problem**: Storing page tables in RAM creates a new performance bottleneck. Every logical memory reference would require *two* physical memory accesses: one to read the page table entry from RAM, and a second to access the actual data. This would effectively halve memory access speed.
  * **Solution 2: Caching (TLB)**: To solve the performance problem, asmall set of fast MMU registers is used as a small, extremely fast, hardware-based cache for recently used page table entries. This cache is often called a **Translation Lookaside Buffer (TLB)**.
      * When the MMU gets a virtual address, it first checks the cache.
      * If the translation is in the cache (a "hit"), the physical address is formed immediately without accessing RAM for the page table.
      * If the translation is not in the cache (a "miss"), the MMU must perform the slow lookup from the page table in RAM, and the new entry is then loaded into the cache, hopefully speeding up future accesses to that same page.
      * While this solves the performance problem, it introduces new complexities, as the operating system must now manage issues like cache hit ratios and invalidating stale cache entries. Effective use of this cache relies heavily on locality of reference. 

***

```text
                 +-----------------+
                 |       CPU       |
                 +-----------------+
                         |
                         | Issues a Virtual Address
                         v
           +---------------------------+
           |      VIRTUAL ADDRESS      |
           | ┌───────────┬─────────┐   |
           | │ Page Num. │ Offset  │   |
           | └───────────┴─────────┘   |
           +---------------------------+
                         |
                         | Sent to MMU for Translation
                         v
┌────────────────────────────────────────────────────────────┐
│            MEMORY MANAGEMENT UNIT (MMU)                    │
│                                                            │
│   Virtual Page #                                           │
│         |                                                  │
│         v                                                  │
│   [ TLB Cache Check ] -----> [ HIT ]                       │
│         |                      | (Fast Path)               │
│         | [ MISS ]             |                           │
│         | (Slow Path)          |                           │
│         v                      '----------------------.    │
│   Lookup from RAM via Page Table                      |    │
│         |                                             |    │
│         '--------------------.                        |    │
│                              |                        |    │
└──────────────────────────────|────────────────────────|────┘
                               |                        |
     (To RAM for Lookup)       |                        | (Translation Complete)
                               v                        v
              +-----------------------------+   +---------------------------+
              |    PAGE TABLE (in RAM)      |   |     PHYSICAL ADDRESS      |
              |-----------------------------|   | ┌───────────┬─────────┐   |
              | Index -> [V|Frame #|Flags]  |   | │ Frame Num.│ Offset  │   |
              +-----------------------------+   | └───────────┴─────────┘   |
                              |                 +---------------------------+
                              |                               |
 (Page Table Entry Fetched)   |                               | Final Address
                              v                               v
┌─────────────────────────────|─────────────────────────┐ +-----------------+
│ MMU (continued)             |                         | |  Physical RAM   |
│                             |                         | +-----------------+
│         ┌-------------------┘                         |
│         |                                             |
│         v                   [ YES ]                   |
│   [ Is Valid Bit 'V' Set? ] --------> To Recombination|
│         |                                             |
│         | [ NO ]                                      |
│         v                                             |
│   Trap to OS (Protection Fault)                       |
│                                                       |
└───────────────────────────────────────────────────────┘
```

***
## 3.4 MMU Operations and Dynamic Memory Management
The Memory Management Unit (MMU) and the operating system perform several ongoing operations to manage memory in a dynamic multitasking environment.

### 3.5 Process Memory Changes
A process's memory requirements often change during its execution. For example, the stack may grow as functions are called, or the process might request more heap memory.
* When a process adds or removes pages from its virtual address space, the operating system directly updates that process's main **page table**, which is stored in RAM.
* Because the MMU's internal cache (TLB) might still hold old, now incorrect, translations for those pages, the OS must execute a **privileged instruction** to flush these stale entries from the MMU's cache, ensuring that future memory accesses will use the updated information.

### 3.6 Context Switching Between Processes
Each process has its own unique virtual address space and its own corresponding page table. When the OS switches the CPU from running one process to another, the MMU's context must be updated.
* The OS executes a privileged instruction that loads a pointer to the **new process's page table** into the MMU hardware.
* The MMU's cache is **flushed** to remove all entries belonging to the previous process. This is critical to prevent the new process from accidentally using the address translations of the previously running process.

### 3.7 Sharing Memory Between Processes
Paging provides a simple and efficient mechanism for processes to share memory, which is commonly used for things like shared code libraries.
* To share a page of memory, the operating system makes the entries in **multiple page tables point to the same physical page frame**.
* This allows different processes to access the identical data in RAM, even though they might use different virtual addresses to do so. This sharing can be set to be **read-only or read/write**.

### 3.8 Handling Extra Pages
In a typical system, the total number of virtual pages needed by all running processes is often far greater than the number of physical page frames available in RAM.
* The solution is to store the "extra" pages that do not fit in RAM on a secondary storage device, such as a flash drive or a hard disk.
* However, there is a fundamental rule: a process can only directly access code or data that is physically located in RAM.
* When the CPU needs to access a page that is currently on disk, the MMU will find no valid translation for it in memory and will trigger a **page fault**. This fault signals the operating system to find the required page on the disk, load it into an available page frame in RAM, and update the page table so the process can resume and finally access its data. This mechanism of loading pages only when they are needed is known as **demand paging**.

---
## 4 Demand Paging
**Demand paging** is the modern approach used to make virtual memory practical. Instead of loading a process's entire memory image into RAM at once, pages are loaded from secondary storage (like a flash drive) only "on demand"—that is, the first time they are referenced.

The key insight behind demand paging is that a process does not need all of its pages to be in memory to run. It only needs the specific pages it is actively referencing at any given time. This allows for much faster program startup and more efficient use of RAM, as pages for obscure features or error-handling routines that are never used in a particular run are never loaded into memory at all.

***
### 4.1 How Demand Paging Works
For demand paging to function, the system hardware and operating system must cooperate:
* The **MMU must support "not present" pages**. The page table must have a way to indicate that a valid page is not currently in a physical page frame.
* When a program references such a page, the MMU **generates an exception**, also known as a page fault.
* The **operating system handles this exception**. It finds the required page on the disk, loads it into an available frame in RAM, updates the page table, and then instructs the MMU to **retry the faulted instruction**.
* A key benefit of this approach is that an **entire process does not need to be in memory to start running**. The system can load just a small subset of pages and fetch the rest as the program demands them.
* The primary challenge of this system is **performance**, as accessing the disk for a page is orders of magnitude slower than accessing RAM.

***
### 4.2 The Page Fault Mechanism
The page fault is the central mechanism that enables demand paging. A **page fault** is a type of exception generated by the MMU when a process attempts to access a virtual address on a page that is not currently present in a physical page frame.

#### Correctness and Page Faults
It is crucial to understand that a page fault is **not an error**, a mistake, or a fatal event that needs to be fixed. Programs should never crash because of page faults. The process itself is unaware that a fault occurred; the mechanism is invisible to the running code, apart from the delay. From a correctness standpoint, a process that experiences one million page faults will produce the exact same result and output as a process that experiences zero page faults. The only difference between the two scenarios is the execution speed.

***
### 4.3 Handling a Page Fault
When a page fault occurs, the operating system follows a precise sequence to resolve it:
1.  The MMU detects the invalid reference and triggers a trap (an exception) to the OS kernel. The faulting user process is halted.
2.  The OS's page fault handler runs. It determines which virtual page was requested and finds its location on the disk (e.g., in the swap file).
3.  The OS finds a free page frame in RAM. If no frames are free, it must run a **page replacement algorithm** (see section 4.3) to select a victim frame.
4.  The OS schedules a disk I/O operation to read the required page from disk into the chosen page frame.
5.  The process that caused the fault is put into a blocked state to wait for the disk I/O to complete. The OS scheduler can run other ready processes during this time.
6.  Once the disk read is finished, the disk controller sends an interrupt to the OS.
7.  The OS updates the faulting process's page table, marking the page as now being present in RAM and pointing to the correct page frame.
8.  The process is moved from the blocked state back to the ready queue.
9.  Eventually, the scheduler runs the process again. The instruction that originally caused the fault is re-executed. This time, the memory access succeeds because the page is now in RAM.

***
### 4.4 Performance Implications of Demand Paging
The performance of a demand-paged system depends entirely on keeping the page fault rate low. This is only possible if the pages a process is about to need are already in RAM the vast majority of the time. The system relies on a principle called **locality of reference** to achieve this.

* **Locality of Reference** is the observed tendency of programs to reuse the same memory locations or access nearby locations in a short period. This behavior is not guaranteed, but it is typical for three main reasons:
    * **Code Locality**: Instructions are usually executed sequentially, and loops cause small sections of code to be executed repeatedly.
    * **Stack Locality**: Data access on the stack is typically confined to the current function's stack frame.
    * **Heap/Data Locality**: Programs often process data structures sequentially, such as iterating through elements in an array or table, leading to accesses on the same page or adjacent pages.

The high cost of page faults comes from two main sources:
* **Process Blocking**: When a page fault occurs, the process that caused it is blocked and cannot continue until the slow disk I/O operation to fetch the page is complete.
* **System Overhead**: Handling a fault consumes significant system resources. The operating system must execute code to handle the exception, schedule the I/O request, potentially context-switch to another process, and later handle the disk interrupt to finalize the operation.

This overhead is directly proportional to the number of page faults. A high fault rate will slow down not just the faulting process, but the entire computer system. The key to good performance is minimizing page faults by having the "right" pages in memory, which is primarily achieved through the use of effective **page replacement algorithms**. While the operating system cannot control which pages a process _requests_, its crucial point of control is in deciding which pages to _eject_ from RAM when a new page must be loaded.

---
## 5. Virtual Memory

Virtual memory is the modern memory management abstraction built upon paging and demand paging. It is used by all major operating systems like Windows, macOS, and Linux.

### 5.1 The Abstraction

Virtual memory provides each process with the illusion that it has its own private, massive, and contiguous address space. This virtual address space can be much larger than the physical RAM available on the machine. The operating system creates this illusion using the hardware and concepts discussed previously.

* Each process is assigned its own vast virtual address space where its segments (code, data, stack) reside.
* The OS then uses **demand paging** and, under certain conditions, **swapping** as the core mechanisms to create this illusion. The system automatically moves pages between RAM and secondary storage (like a flash drive) as they are needed.
* The central challenge for the OS is to successfully create and manage this abstraction without the user noticing the underlying mechanics.

### 5.2 The Performance Goal
A primary goal of the virtual memory abstraction is not just to provide more memory, but to do so at a high speed. The system is designed to make memory access feel as if it is running at the speed of RAM, even though some data resides on the much slower disk. This performance goal is what makes the efficiency of the underlying technologies, especially **page replacement algorithms**, so critical.

---
## 6. Page Replacement Algorithms

When a page fault occurs and all page frames are already in use, the OS must decide which page to evict from RAM to make room for the new one. The choice of which page to replace is critical for system performance.

  * **Goal**: To choose a victim page that will not be needed for the longest possible time, thus minimizing the future page fault rate.
  * **Optimal (OPT/MIN) Algorithm**: This algorithm replaces the page that will be referenced furthest in the future. It is provably optimal but impossible to implement in practice because it requires predicting the future. It is used as a theoretical benchmark.
  * **Approximating OPT**: Since the optimal algorithm is not feasible, practical algorithms aim to approximate its behavior. They do this by relying on the **principle of locality of reference**, using a program's past behavior to predict its future memory needs. The most common approach based on this principle is Least Recently Used (LRU).

***
### 6.1 Naive LRU
The logic of a naive or "true" LRU algorithm is to replace the page that has not been used for the longest time. A conceptual implementation would be:
1.  On every memory access to a page, record a precise timestamp.
2.  When a page fault occurs, search through the timestamps for all pages currently in memory.
3.  Select the page with the oldest timestamp as the victim.

#### 6.1.1 LRU Example

Consider 4 page frames and the reference stream: `a, b, c, d, a, b, d, e, f, a, b, c, d, a, e, d`.

| Ref   | Frame 0 | Frame 1 | Frame 2 | Frame 3 | Faults/Notes                |
|:------|:--------|:--------|:--------|:--------|:----------------------------|
| **a** | **a** |         |         |         | Fault (Load)                |
| **b** | a       | **b** |         |         | Fault (Load)                |
| **c** | a       | b       | **c** |         | Fault (Load)                |
| **d** | a       | b       | c       | **d** | Fault (Load)                |
| **a** | a       | b       | c       | d       | Hit                         |
| **b** | a       | b       | c       | d       | Hit                         |
| **d** | a       | b       | c       | d       | Hit                         |
| **e** | a       | b       | **e** | d       | Fault (Replace `c`, LRU)    |
| **f** | **f** | b       | e       | d       | Fault (Replace `a`, LRU)    |
| **a** | f       | **a** | e       | d       | Fault (Replace `b`, LRU)    |
| **b** | f       | a       | e       | **b** | Fault (Replace `d`, LRU)    |
| **c** | f       | a       | **c** | b       | Fault (Replace `e`, LRU)    |
| **d** | **d** | a       | c       | b       | Fault (Replace `f`, LRU)    |
| **a** | d       | a       | c       | b       | Hit                         |
| **e** | d       | a       | **e** | b       | Fault (Replace `c`, LRU)    |
| **d** | d       | a       | e       | b       | Hit                         |

*Total Faults with True LRU: 4 loads + 7 replacements = 11 faults.*

#### 6.2.2 LRU Downfalls
Implementing true LRU requires recording a precise timestamp on every memory reference, which creates an unacceptable performance trade-off, so the algorithm is not used in practice.

* **Hardware Cost**: Implementing LRU in hardware would require the MMU to update a timestamp on every memory access. This operation would slow the MMU's critical path, which conflicts with its primary goal of performing fast translations. Hardware designers therefore provide only minimal support, such as a single `read` or `written` bit per page, which is far less costly than maintaining full timestamps.
* **Software Cost**: A software-only implementation is even less feasible, as it would require an OS trap on every memory reference to update a timestamp data structure. The resulting overhead of executing multiple instructions for every single user instruction would slow the entire system to a crawl.

These high costs necessitate the use of a **cheap software surrogate for LRU**. Operating systems therefore use efficient approximation algorithms like the **Clock Algorithm**, which provides most of LRU's benefits at a fraction of the cost.

***
### 6.3 Clock Algorithm
To avoid the high cost of true LRU, operating systems use an efficient surrogate like the **Clock Algorithm** (also known as the Second-Chance Algorithm). It approximates LRU's behavior using a single `reference bit` per page frame and a circular scanning mechanism, providing most of the benefits at a fraction of the cost. The "clock hand" in the Clock Algorithm is a direct implementation of a guess pointer. 

***
#### 6.3.1 Mechanism:
1.  All page frames are arranged in a circular buffer with a "clock hand" pointer.
2.  Each frame has a `reference bit` (or `use bit`), which is set to 1 by the MMU hardware whenever the page is accessed.
3.  On a page fault, the OS inspects the frame pointed to by the clock hand.
4.  If its reference bit is 1, the OS clears the bit to 0 (giving it a "second chance") and advances the clock hand to the next frame.
5.  If its reference bit is 0, that frame is chosen as the victim. The new page is loaded into it, and the hand is advanced. The search always resumes from where the clock hand left off.

***
#### 6.3.2 Clock Algorithm Example

Consider 4 page frames and the reference stream: `a, b, c, d, a, b, d, e, f, a, b, c, d, a, e, d`.

| Ref   | Frame 0 | Frame 1 | Frame 2 | Frame 3 | Faults/Notes          |
|:------|:--------|:--------|:--------|:--------|:----------------------|
| **a** | **a** |         |         |         | Fault (Load)          |
| **b** | a       | **b** |         |         | Fault (Load)          |
| **c** | a       | b       | **c** |         | Fault (Load)          |
| **d** | a       | b       | c       | **d** | Fault (Load)          |
| **a** | a       | b       | c       | d       | Hit                   |
| **b** | a       | b       | c       | d       | Hit                   |
| **d** | a       | b       | c       | d       | Hit                   |
| **e** | a       | b       | **e** | d       | Fault (Replace `c`)   |
| **f** | **f** | b       | e       | d       | Fault (Replace `a`)   |
| **a** | f       | **a** | e       | d       | Fault (Replace `b`)   |
| **b** | f       | a       | **b** | d       | Fault (Replace `e`)   |
| **c** | f       | a       | b       | **c** | Fault (Replace `d`)   |
| **d** | **d** | a       | b       | c       | Fault (Replace `f`)   |
| **a** | d       | a       | b       | c       | Hit                   |
| **e** | d       | a       | **e** | c       | Fault (Replace `b`)   |
| **d** | d       | a       | e       | c       | Hit                   |

*Total Faults with Clock: 4 loads + 7 replacements = 11 faults. In this specific example, the fault count is the same as LRU, but the pages replaced are different, demonstrating it is an approximation.*

Overall, Clock Algorithm's decisions are 98% as good as True LRU but can be done for 1% of the cost. 

---
## 7. Managing Memory in a Multiprogramming Environment
In a multitasking system with many competing processes, the page memory management strategy is critical. A naive approach can cause a process to lose its essential pages from memory while it is inactive, leading to a storm of page faults when it resumes execution. The challenge is to efficiently allocate page frames among all processes to prevent this. Three main strategies exist.

***
### 7.1 Single Global Pool
This strategy treats all page frames as a single resource pool. When a page fault occurs, a global replacement algorithm, like an LRU approximation, evicts the least recently used page in the *entire system*, regardless of which process owns it.

* **Critical Flaw**: This approach interacts poorly with process scheduling. A process that has been waiting for a long time will not have accessed its pages, making them prime candidates for replacement. When the process finally runs, it will immediately fault on the pages it needs, which were just evicted.

***
### 7.2 Per-Process Fixed Allocation
This strategy attempts to solve the problem of the global pool by providing process isolation. It partitions the available page frames and assigns a **fixed number** of frames to each running process. For example, if there are *N* processes, each might be allocated 1/*N* of the total frames.

When a process experiences a page fault, it runs a replacement algorithm (like an LRU approximation) **only on the pages within its own designated set of frames**.

* **Advantage**: This approach provides strong isolation. Other processes cannot "steal" pages from an inactive process. This means that when a process gets its turn to run again, it is more likely to find its essential pages still in memory, avoiding the storm of page faults seen with the global pool strategy.

* **Critical Flaw**: While it solves the isolation problem, a *fixed* allocation is too rigid and inefficient. The core issue is that a static allocation cannot adapt to the diverse and dynamic memory needs of processes:
    * **Different Processes, Different Needs**: Different programs exhibit different degrees of locality and have vastly different memory footprints. A database server might need many page frames, while a simple text editor needs very few. A fixed allocation will starve the memory-intensive process (causing it to fault constantly) while wasting frames on the process with small needs.
    * **Needs Change Over Time**: Even a single process's memory requirements change during its execution. It might need many pages while initializing, but very few during its main processing loop.

This problem is analogous to why simple round-robin scheduling is inefficient. Just as processes have different "natural" CPU burst lengths that a dynamic scheduler like MLFQ can adapt to, they also have different and changing memory needs. A static, fixed allocation of page frames is fundamentally unable to accommodate this dynamic behavior.

---
## 8. Working Set Model
The failures of both global and fixed-allocation models lead to the need for a dynamic and customized allocation strategy. The **working set model** provides this by giving each process a customized number of page frames matched to its needs, and dynamically adjusting this number over time.

***
### 8.1 The Working Set Concept
The **working set** of a process is defined as the set of pages it has used in a fixed-length sampling window in the immediate past. The core idea is to allocate enough page frames to each process to hold its current working set. This ensures that the pages a process is actively using remain in memory, minimizing page faults.

***
### 8.2 The Natural Working Set Size and Performance
The number of page frames allocated to a process (its working set size) has a direct impact on its page fault rate. This relationship is not linear:

  * **Too Small a Working Set**: If a process has insufficient frames to hold its working set, it will fault constantly as needed pages replace one another continuously. This is the region of thrashing.
  * **Too Large a Working Set**: Giving a process more frames than its current working set size provides little marginal benefit. The extra frames will go unused, wasting a valuable system resource.
  * **The "Sweet Spot"**: The ideal allocation is at the "knee of the curve," where the process has just enough frames to hold its working set, minimizing page faults without wasting memory.

***
### 8.3 The Optimal Working Set
Theoretically, the **optimal working set** for a process is the exact number of pages it needs to execute through its next time slice without incurring any page faults. If a process could be run with this precise number of frames, it would achieve maximum performance for that execution interval.

However, just like the optimal page replacement algorithm, determining the optimal working set size in advance is impossible because it would require predicting the future memory accesses of the process. The operating system cannot know what pages a process will need before it runs.

#### 8.3.1 Approximating the Optimal
Since the true optimal size cannot be known, the operating system must approximate it by observing a process's past behavior. The goal is to dynamically adjust a process's page frame allocation to match its needs as closely as possible.

* **What if the allocation is too small?** If a process runs with fewer frames than its optimal working set, it will experience continuous page faults as needed pages replace one another. The process will run very slowly, and the system will spend all its time handling faults.
* **What if the allocation is too large?** If a process has many more frames than its optimal working set, the extra frames will go unused. This is wasteful, as those frames could have been allocated to another process, preventing it from faulting.

***
### 8.4 Implementing Working Sets

The operating system does not try to guess which specific pages should be in the working set. Instead, it relies on **demand paging** to let the process itself reveal its needs.

1.  **Observe Paging Behavior**: The OS monitors the page fault rate (faults per unit time) for each process.
2.  **Adjust Working Set Size**: Based on this observation, the OS dynamically adjusts the number of page frames assigned to each process:
      * If a process's fault rate is **too high**, its working set has likely grown, and the OS needs to give it more page frames.
      * If a process's fault rate is **very low**, it may have more frames than it needs, and some can be taken away to be used by other processes.
3.  **Page Stealing Algorithms**: To reallocate frames, the system uses a **page stealing algorithm**. When one process needs more frames, the algorithm will "steal" a page frame from another process that appears to have more than it needs (i.e., one with a very low fault rate). An algorithm like **Working Set-Clock** is used to track page usage and identify the best candidates for stealing.

This dynamic approach ensures that processes that need more pages tend to get them, while processes that are not using their allocated pages tend to lose them, leading to an efficient distribution of memory resources across the system.

---
## 9. Thrashing
While the working set model is an effective strategy, it can lead to a catastrophic performance collapse known as **thrashing** if the system becomes overcommitted. Thrashing is a state where the system spends all of its time moving pages between RAM and disk and performs almost no useful computation. When a system thrashes, all processes run extremely slowly, and the computer becomes effectively unusable.

***
### 9.1 The Cause of Thrashing
Thrashing occurs when the **sum of the working sets of all active processes exceeds the total number of available physical page frames**.

When this happens, no single process can be allocated enough page frames to hold its full working set. The system enters a vicious cycle:
1.  A process is scheduled to run, but its working set does not fit in its allocated frames.
2.  It almost immediately incurs a **page fault**.
3.  To make room for the needed page, the operating system's page stealer takes a frame from another process.
4.  However, the "victim" process also needed that frame for its own working set.
5.  When the victim process gets its turn to run, it immediately faults on the page that was just stolen.
6.  This cycle repeats across all processes. Every process steals pages that other processes need, causing a constant cascade of page faults throughout the system.

***
### 9.2 The Consequences
The result of this constant faulting is a complete system breakdown:
* The **disk becomes overloaded** with page-in and page-out requests.
* Processes are constantly **blocked**, waiting for the disk to service their page faults.
* Because no process is ever ready to do computational work, **CPU utilization plummets**. The system is perpetually busy with I/O but accomplishes nothing.

This state of high paging activity and low CPU utilization is the defining characteristic of thrashing. Generally, once a system begins to thrash, it will not recover on its own.

***
### 9.3 Preventing and Stopping Thrashing
To stop thrashing, the operating system must address the root cause: there are too many processes competing for too few resources. Since adding more physical memory is not a dynamic option, the only viable solution is to **reduce the level of multiprogramming**.

1.  **Identify the Problem**: The OS detects thrashing by observing the high page fault rate and low CPU utilization.
2.  **Select a Victim Process**: The OS will choose one or more "victim" processes to suspend.
3.  **Swap the Process Out**: The chosen process is **swapped out entirely**. All of its pages are removed from RAM and written to the disk. This is a more drastic action than normal page replacement.
4.  **Reallocate Frames**: The page frames that are freed up are then redistributed among the remaining active processes.
5.  **Resume Normal Operation**: With fewer processes competing for memory, the remaining processes can now hopefully hold their full working sets in RAM, allowing them to run efficiently and stopping the thrashing cycle.

The swapped-out process will not run for a while, but this is preferable to the entire system being unusable. The OS can maintain fairness by periodically rotating which processes are swapped out.

---
### 5.5 Clean vs. Dirty Pages

The state of a page impacts the cost of replacing it.

  * **Dirty Page**: A page in memory that has been written to since it was loaded from disk. Its contents are newer than the copy on disk.
  * **Clean Page**: A page in memory that has not been modified. An identical, valid copy exists on disk.

The MMU hardware helps track this with a **dirty bit** (or write bit) for each page, which it sets on any write operation to that page.

  * **Impact on Replacement**:

      * Replacing a **clean page** is cheap: the OS can simply overwrite its frame because a valid copy is already on disk.
      * Replacing a **dirty page** is expensive: the OS must first perform a disk write to save the modified contents back to the disk before the frame can be reused for the new page. This doubles the I/O work for the page fault (one write, one read).

  * **Preemptive Page Laundering**: To increase flexibility, the OS can run a background daemon during idle periods. This process finds dirty pages and writes them to disk *preemptively*, before they are chosen as replacement victims. This "laundering" converts them into clean pages. When a page fault later occurs during a busy period, there is a higher chance of finding a clean page to replace, avoiding the expensive write-before-read penalty.
