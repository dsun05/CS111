# Memory Management in Modern Operating Systems

## 1. Introduction to Memory Management Challenges

### 1.1 Initial Approaches

- **Fixed Partitions**
  - Memory is divided into fixed-size blocks.
  - Limitations:
    - Internal fragmentation: unused memory within an allocated block.
    - Limited scalability: cannot handle many or large processes.

- **Dynamic Partitions**
  - Memory allocated based on process needs.
  - Benefits:
    - Reduces internal fragmentation.
  - Drawbacks:
    - Suffers from external fragmentation: small unusable holes form over time.
    - Still requires contiguous memory space for each segment.

### 1.2 Fragmentation Problems

| Type               | Definition                                                                 | Example                          |
|--------------------|-----------------------------------------------------------------------------|----------------------------------|
| Internal Fragmentation | Occurs when allocated memory may be larger than the requested size.         | 512KB allocated, 400KB used.     |
| External Fragmentation | Memory is available but in small, non-contiguous chunks. Cannot be used effectively. | Multiple 2KB holes, but need 4KB |

## 2. Solving Fragmentation: Segmentation and Swapping

### 2.1 Segmentation with Registers

- **Segmentation**: Dividing process memory into `code`, `stack`, `data` segments.
- Segment registers: Allow relocation of entire segments.
- Limitation: Still require contiguous memory per segment.

### 2.2 Swapping

- **Swapping**: Temporarily move entire process memory to disk.
- Benefits:
  - Frees RAM for other processes.
  - Allows RAM usage beyond physical limits (one process at a time).
- Costs:
  - Extremely slow context switches when processes are swapped in/out.
  - High disk I/O overhead.
  - Does not scale for multitasking with high memory demands.

## 3. Paging

### 3.1 Core Concept

- **Paging**: Divide physical memory and process memory into fixed-size blocks.
  - RAM → **Page Frames** (e.g., 4KB).
  - Process memory → **Pages** of same size.
- Goal: Eliminate need for contiguous memory allocation.

### 3.2 Address Translation

| Term | Description                              |
|------|------------------------------------------|
| Virtual Address | An address used by the process.            |
| Physical Address | Actual address in RAM.                    |
| Page             | Virtual memory block (e.g., 4KB).         |
| Page Frame       | Physical memory block (same as page size).|

- Translation from Virtual Address to Physical Address involves:
  - Splitting virtual address into **Page Number** and **Offset**.
  - Mapping page number to page frame via **Page Table**.

### 3.3 Fragmentation with Paging

- **Internal Fragmentation** occurs only in last page of a segment.
- **External Fragmentation** is eliminated (fixed-size allocations).

| Aspect                | Result                                 |
|------------------------|------------------------------------------|
| Internal Fragmentation | Average 0.5 page per segment.             |
| External Fragmentation | None (fixed allocation size).           |

### 3.4 Performance Cost of Translation

- Every memory reference needs address translation.
- Require:
  - **Hardware support**: Translation must be fast.
  - **Memory Management Unit (MMU)**: Hardware component for translation.

## 4. Memory Management Unit (MMU)

### 4.1 MMU Functionality

- Translates virtual addresses → physical addresses.
- Uses **Page Tables** maintained per process.
- Needs to perform translation at instruction speed.

### 4.2 Page Table Structure

- Maps Virtual Page Number → Page Frame Number.
- Includes:
  - **Valid bit**: Indicates valid mapping or if page is in memory.

### 4.3 MMU Hardware

| Phase            | Description                                                 |
|------------------|-------------------------------------------------------------|
| Early MMUs       | External chips between CPU & memory bus.                    |
| Modern MMUs      | Integrated into CPU chip for speed.                         |

### 4.4 Page Table Storage

- **Page Tables in RAM**:
  - Cannot store all mappings in CPU registers.
  - E.g., 64-bit system with 4KB pages = 2^52 entries possible.

- **Solution**: MMU uses a **cache**:
  - **Translation Lookaside Buffer (TLB)**: Holds recently-used translations.
  - Avoids 2 memory accesses for each instruction.

### 4.5 Context Switching and MMU

- Processes have separate virtual addresses.
- Upon context switch:
  - Must **flush or switch** page table mappings.
  - Solutions for TLB flushing:
    - Explicit flush on context switch.
    - TLB tagging by process ID.

## 5. Virtual Memory

- **Virtual Memory**: Abstraction allowing processes to use more memory than physically available.

### 5.1 Demand Paging

- **Demand Paging**: Pages are loaded into memory only when referenced.
- If not in memory:
  - **Page Fault** occurs.
  - OS loads page from disk → updates page table.

### 5.2 Page Fault Handling

#### Steps:

| Step | Description |
|------|-------------|
| 1    | MMU detects a missing page → signals OS (trap/exception). |
| 2    | OS finds page location on disk via page table. |
| 3    | Schedules disk I/O to read page into a free page frame. |
| 4    | Updates page table with new frame info. |
| 5    | Unblocks process → resume instruction that caused fault. |

> Note: Page faults are not errors. They are transparency mechanisms to enable virtual memory.

### 5.3 Impact of Page Faults

- Correctness unaffected.
- Performance heavily affected.
- Page fault = slow disk access + OS overhead.

## 6. Locality of Reference

### 6.1 Types of Locality

- **Temporal Locality**: Reuse of recent data (e.g., loop variables).
- **Spatial Locality**: Use of adjacent data (e.g., traversing arrays).

### 6.2 Sources

- **Code**: Sequential instruction execution and loops.
- **Stack**: Execution stays in a few recent functions.
- **Heap/Data**: Structured access (e.g., arrays, tables).

### 6.3 Importance

- High locality = fewer page faults.
- Virtual memory performance depends on locality.

## 7. Page Replacement

### 7.1 When to Replace

- When a page fault occurs and no free frame is available.

### 7.2 Optimal Replacement Algorithm

- Replace page that won’t be used for the longest future time.
- Not feasible: requires future knowledge (oracle-based).

### 7.3 Heuristic Algorithms

| Algorithm              | Description                                              |
|------------------------|----------------------------------------------------------|
| Random                 | Choose any page randomly. Poor performance.              |
| FIFO                   | Remove oldest page. Poor approximation, no recency info. |
| LFU (Least Frequently Used) | Based on usage count. Can misinterpret recent usage.     |
| LRU (Least Recently Used)   | Evict page not used for the longest time. Good approximation. |
| Clock Algorithm        | Efficient LRU approximation using reference bits.        |

### 7.4 Clock Algorithm Details

- Pages arranged in circular list.
- MMU sets **reference bit** on use.
- Clock hand checks:
  - If bit = 1 → clear it, move on.
  - If bit = 0 → replace this page.

Example:

| Page Frame | Reference Bit | Clock Choice |
|------------|----------------|--------------|
| 0          | 1              | Cleared      |
| 1          | 1              | Cleared      |
| 2          | 0              | Replace      |

### 7.5 Comparison

- **True LRU**: More accurate, expensive to implement.
- **Clock Algorithm**: Efficient, near-LRU performance.

## 8. Working Sets and Page Allocation

### 8.1 The Working Set Model

- **Working Set**: Collection of pages a process needs with low fault rate.
- Goal:
  - Allocate optimal working set size.
  - Adjust based on recent page fault metrics.

### 8.2 Allocation Strategies

| Strategy       | Description                                            |
|----------------|--------------------------------------------------------|
| Global Pool    | All pages in shared pool. Leads to poor performance.   |
| Fixed Allocation | Equal pages per process. Doesn't reflect true need.    |
| Working Set Allocation | Adaptive. Allocates pages based on recent usage.    |

### 8.3 Working Set Tuning

- Processes tracked for page fault frequency.
- Page Stealing:
  - OS periodically reviews allocation.
  - Steals from low-fault processes for high-fault ones.

### 8.4 Thrashing

- **Thrashing**: Constant page faulting due to insufficient page frames.
- Causes:
  - Over-committed memory.
  - Insufficient page frame allocation.

#### Solution:

- **Swapping Out a Process**:
  - Swap entire process state to disk.
  - Redistribute its pages to reduce overall load.
  - Optionally rotate swapped-out processes.

## 9. Clean vs Dirty Pages

| Type   | Description                                                                         | Handling                  |
|--------|-------------------------------------------------------------------------------------|---------------------------|
| Clean  | In-memory data is unchanged from disk copy.                                        | No write needed on eviction. |
| Dirty  | Modified since loading. Must write to disk before eviction.                        | Write then evict.         |

### 9.1 Preemptive Page Cleaning (Page Laundry)

- When system idle:
  - OS writes dirty pages back to disk.
  - Converts them to clean pages → easier future eviction.
  - Improves flexibility in page replacement.

## 10. Summary

This lecture covered the evolution of memory management techniques in operating systems, culminating in the modern use of virtual memory. We examined the inefficiencies of early partitioning schemes and how paging overcomes them by eliminating the requirement for contiguous memory and significantly reducing fragmentation. The lecture introduced the concept of the MMU and its role in performing fast address translation, highlighted the importance of demand paging for handling large memory requirements, and detailed the mechanics and implications of page faults. Strategies for efficient page replacement were explored, including the optimal algorithm, LRU, and the practical clock algorithm. Finally, the concept of working sets was introduced to adaptively manage memory allocation for multiple concurrently running processes, emphasizing strategies to avoid thrashing and improve system-wide performance through techniques like preemptive page cleaning and process swapping.