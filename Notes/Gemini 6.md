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
1.  The MMU hardware traps to the operating system kernel, halting the faulting process.
2.  The OS's page fault handler identifies which page was requested and finds its location on the disk.
3.  The OS finds a free page frame in RAM. If none are free, a page replacement algorithm is run to select a "victim" page to evict.
4.  The OS schedules an I/O operation to read the required page from the disk into the chosen frame. The process is blocked while this occurs, allowing other processes to run.
5.  Once the disk read is complete, the OS updates the process's page table to reflect that the page is now in memory, mapping it to the correct frame.
6.  The process is unblocked and placed back on the ready queue. When it runs again, the instruction that caused the fault is re-executed and now succeeds.

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

Virtual memory provides each process with the illusion that it has its own private, massive, and contiguous address space. This virtual address space can be much larger than the physical RAM available on the machine. The OS and MMU work together to manage the mapping between the large virtual address space and the limited physical RAM, hiding the complexity from the programmer.

### 5.2 Demand Paging

Demand paging is the key mechanism that makes virtual memory efficient and practical.

  * **Concept**: Instead of loading all of a process's pages into RAM when it starts, pages are only loaded from the disk "on demand," i.e., the first time they are referenced. Many pages of a large program (e.g., error handling code, obscure features) may never be needed and are therefore never loaded into memory, saving RAM and speeding up program startup.

#### Page Faults

A **page fault** is a hardware exception generated by the MMU when a process attempts to access a page that is marked as valid in its page table but is not currently present in a physical page frame (i.e., its valid bit is off, or a "present" bit is clear). It's crucial to understand that a page fault is **not a programming error**; it is a normal, albeit slow, event in a demand-paged system that triggers the OS to load the required page from disk.

#### Handling a Page Fault

The operating system follows a precise sequence to handle a page fault:

1.  The MMU detects the invalid reference and triggers a trap (an exception) to the OS kernel. The faulting user process is halted.
2.  The OS's page fault handler runs. It determines which virtual page was requested and finds its location on the disk (e.g., in the swap file).
3.  The OS finds a free page frame in RAM. If no frames are free, it must run a **page replacement algorithm** (see section 4.3) to select a victim frame.
4.  The OS schedules a disk I/O operation to read the required page from disk into the chosen page frame.
5.  The process that caused the fault is put into a blocked state to wait for the disk I/O to complete. The OS scheduler can run other ready processes during this time.
6.  Once the disk read is finished, the disk controller sends an interrupt to the OS.
7.  The OS updates the faulting process's page table, marking the page as now being present in RAM and pointing to the correct page frame.
8.  The process is moved from the blocked state back to the ready queue.
9.  Eventually, the scheduler runs the process again. The instruction that originally caused the fault is re-executed. This time, the memory access succeeds because the page is now in RAM.

### 5.3 Page Replacement Algorithms

When a page fault occurs and all page frames are already in use, the OS must decide which page to evict from RAM to make room for the new one. The choice of which page to replace is critical for system performance.

  * **Goal**: To choose a victim page that will not be needed for the longest possible time, thus minimizing the future page fault rate.
  * **Optimal (OPT/MIN) Algorithm**: This algorithm replaces the page that will be referenced furthest in the future. It is provably optimal but impossible to implement in practice because it requires predicting the future. It is used as a theoretical benchmark.
  * **Least Recently Used (LRU)**: LRU is a common approximation of the OPT algorithm. It works on the assumption of locality of reference: if a page hasn't been used in a long time, it is unlikely to be used in the near future. Therefore, LRU replaces the page that has the oldest last-access time.
      * **Implementation**: True LRU is prohibitively expensive. It would require the system to maintain a timestamp for every page and update it on every single memory reference, and then search all timestamps to find the oldest on every page fault.

#### LRU Example

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

  * **Clock Algorithm (Second-Chance Algorithm)**: A practical and efficient approximation of LRU that avoids the high overhead of timestamps.
      * **Mechanism**:
        1.  All page frames are arranged in a circular buffer with a "clock hand" pointer.
        2.  Each frame has a `reference bit` (or `use bit`), which is set to 1 by the MMU hardware whenever the page is accessed.
        3.  On a page fault, the OS inspects the frame pointed to by the clock hand.
        4.  If its reference bit is 1, the OS clears the bit to 0 (giving it a "second chance") and advances the clock hand to the next frame.
        5.  If its reference bit is 0, that frame is chosen as the victim. The new page is loaded into it, and the hand is advanced. The search always resumes from where the clock hand left off.

#### Clock Algorithm Example

This trace follows the example presented in the lecture slide.

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

### 5.4 Managing Memory in a Multiprogramming Environment

#### Working Set Model

A fixed page allocation per process is inflexible. The **working set model** is a dynamic strategy where the OS tries to keep the set of pages that a process is "working with" in memory.

  * **Working Set**: The set of pages a process has referenced in the most recent time interval, Δ. The goal of the OS is to ensure a process's full working set is in memory before letting it run.
  * **Dynamic Adjustment**: The OS monitors the page fault rate of each process.
      * If a process's fault rate is too high, its working set has likely grown, and the OS will allocate it more page frames (often by "stealing" them from a process with a low fault rate).
      * If a process's fault rate is very low, it may have more frames than it needs, and some can be taken away for other processes.

#### Thrashing

**Thrashing** is a condition where a system's performance collapses because it is spending all its time paging (swapping pages between RAM and disk), and no useful computation is being done.

  * **Cause**: Thrashing occurs when the sum of the working sets of all active processes is greater than the total amount of physical RAM. This leads to a vicious cycle: a process faults, steals a frame from another process, which then faults, steals a frame from yet another process, and so on. The disk queue becomes overloaded, and CPU utilization plummets because processes are constantly blocked waiting for I/O.
  * **Solution**: To stop thrashing, the OS must reduce the level of multiprogramming. It will select one or more "victim" processes and swap them out entirely to disk, suspending them. This frees up enough page frames for the remaining processes to have their full working sets in memory, allowing them to run efficiently.

### 5.5 Clean vs. Dirty Pages

The state of a page impacts the cost of replacing it.

  * **Dirty Page**: A page in memory that has been written to since it was loaded from disk. Its contents are newer than the copy on disk.
  * **Clean Page**: A page in memory that has not been modified. An identical, valid copy exists on disk.

The MMU hardware helps track this with a **dirty bit** (or write bit) for each page, which it sets on any write operation to that page.

  * **Impact on Replacement**:

      * Replacing a **clean page** is cheap: the OS can simply overwrite its frame because a valid copy is already on disk.
      * Replacing a **dirty page** is expensive: the OS must first perform a disk write to save the modified contents back to the disk before the frame can be reused for the new page. This doubles the I/O work for the page fault (one write, one read).

  * **Preemptive Page Laundering**: To increase flexibility, the OS can run a background daemon during idle periods. This process finds dirty pages and writes them to disk *preemptively*, before they are chosen as replacement victims. This "laundering" converts them into clean pages. When a page fault later occurs during a busy period, there is a higher chance of finding a clean page to replace, avoiding the expensive write-before-read penalty.
