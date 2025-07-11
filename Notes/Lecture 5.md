# 1. Introduction to Memory Management

Memory management is a key responsibility of any operating system (OS). It involves efficiently and securely allocating system memory (RAM) among multiple running processes.

## 1.1 Purpose and Goals

- **Purpose**: Efficient and secure allocation of limited RAM across numerous processes.
- **Goals**:
  - Maintain process isolation and independence.
  - Ensure efficient use of memory.
  - Minimize overhead for allocation/deallocation.
  - Protect memory from improper access (protection and isolation).

## 1.2 RAM in the Von Neumann Model

- RAM stores:
  - Program instructions.
  - Data.
  - Stack information for function return and variable storage.
- Every process and the OS kernel require access to RAM.
- RAM is:
  - Expensive.
  - Limited in size.
  - A crucial shared resource on modern systems.

# 2. Memory Allocation Basics

## 2.1 Physical vs. Virtual Memory

- **Physical Address**: Real memory location on RAM chip.
- **Virtual Address**: Address used by processes; translated to physical addresses.

## 2.2 Key Requirements of Memory Management

- Each process requires its own isolated, protected memory (address space).
- Cannot predict future memory requirements perfectly.
- Memory writes to an address must persist and not be overwritten unexpectedly.
- Normally, actual RAM required > available RAM — necessitates sharing and optimization.

# 3. Early Memory Management Strategies

## 3.1 Fixed Partition Allocation

### Description

- Divide RAM into fixed-size partitions.
- Assign partitions to processes.
- One partition per process or a limited number of partitions per process.

### Advantages

- Simple implementation.
- Fast allocation (e.g., using a bitmap).
- Works well in constrained environments or embedded systems.

### Disadvantages

- **Internal fragmentation**: Memory is wasted inside allocated partitions that are not fully used.
- Not relocatable.
- Inefficient for large numbers or variable-sized processes.

### Example

| Process | Required Memory | Partition Size | Unused Memory (Wasted) |
|---------|------------------|----------------|------------------------|
| A       | 6MB              | 8MB            | 2MB                    |
| B       | 3MB              | 4MB            | 1MB                    |
| C       | 2MB              | 4MB            | 2MB                    |
| **Total Waste**                             |                        | 5MB (31%)               |

#### Key Concept: Internal Fragmentation

- Caused by mismatch between partition size and actual need.
- Wasted memory cannot be used by others.

## 3.2 Dynamic Partition Allocation

### Description

- Allocate exactly the amount of memory a process requests.
- Contiguous allocation per process.
- Variable-sized partitions dependent on process needs.

### Advantages

- No internal fragmentation.
- More flexible than fixed partitions.

### Disadvantages

- More complex to manage (needs free list structures).
- **External fragmentation**: unusable free memory chunks scattered in memory.
- Not relocatable.
- Not easily expandable.

#### Memory Allocation Data Structure

- **Free List**: Linked list format, keeping track of start addresses and sizes.

#### Free Chunk Carving

- Process requests `n` bytes.
- Search free list for a suitable chunk.
- Carve off exactly `n` bytes.
- Remaining fragment returned to free list.

#### Key Concept: External Fragmentation

- Over time, numerous small chunks of free memory accumulate.
- Even with sufficient total space, allocations may fail due to lack of contiguous memory.

#### Expansion and Relocation Problems

- A process cannot expand its memory unless adjacent space is free.
- Relocation (moving partition) is difficult due to internal pointers.

### Coalescing

- When adjacent free chunks are found, merge them into larger chunks.
- Typically done during deallocation.

```text
[Free][Used A][Free][Used B][Free] 
↓ Coalescing
[Free..........][Used B][Free]
```

### Allocation Algorithms

| Algorithm  | Description |
|------------|-------------|
| **Best Fit** | Choose the smallest free chunk that satisfies the request. May cause many small, unusable fragments. |
| **Worst Fit**| Choose the largest chunk. Leaves larger fragments for future requests. |
| **First Fit**| Choose the first sufficiently large chunk. Fast but fragments beginning of memory. |
| **Next Fit** | Like First Fit, but resumes search after previous allocation point. Helps distribute fragmentation. |

#### Guest Pointers

- A pointer used to guess the best starting point for memory search.
- Efficient in practice and used in multiple system design scenarios.

# 4. Buffer Pools

## 4.1 Motivation

- Apps (and OS) often request the same-sized memory chunks repeatedly.
- Examples:
  - Network buffers.
  - Disk cache buffers.

## 4.2 Design

- Preallocate a memory region into fixed-size buffers.
- Maintain bitmap to manage allocations.
- Only allocate and deallocate whole buffers.

### Advantages

- No internal or external fragmentation for exact-size matches.
- Fast allocation/deallocation.

### Disadvantages

- Wastes memory if allocation requests don't match buffer size.
- Only supports popular/requested sizes efficiently.

## 4.3 Dynamic Resizing

- Monitor buffer pool use.
- Add or release buffers to optimize pool size.
- Allocations outside buffer pool go through general allocation.

### Buffer Pool Management Table

| Buffer Size | Allocations (Count) | Recommendation         |
|-------------|----------------------|------------------------|
| 4K          | 10000                | Maintain/Expand Pool   |
| 1K          | 2                    | Shrink/Remove Pool     |

# 5. Memory Leaks

## 5.1 Description

- Allocated memory is not deallocated due to bugs or lost references.
- Memory becomes unusable while remaining allocated.

## 5.2 Symptoms

- Application performance degradation over time.
- Memory usage increases steadily.
- Can crash long-running services.

## 5.3 Solutions: Garbage Collection

### How it Works

- Identify unreachable memory objects.
- Reclaim them for reuse.

### Implementation Strategy

1. Trace all reachable objects.
2. Mark as alive.
3. Reclaim anything unmarked.

### Challenges

- Difficult in systems without type safety.
- Not all references are obvious.
- Works best in:
  - Object-oriented languages.
  - Systems using well-defined object descriptors.

# 6. Memory Compaction

## 6.1 Problem

- External fragmentation causes large total free memory but no large contiguous space.

## 6.2 Solution

- **Memory compaction**: move allocated memory segments to one part of RAM to coalesce free space into a single large region.

```text
Before Compaction:
[Used][Free][Used][Free][Used]

After Compaction:
[Used][Used][Used][Free........]
```

## 6.3 Requirements

- Ability to **relocate** memory contents.
- Modify internal pointers (difficult if using physical addresses).

## 6.4 Limitations

- Slow: requires copying.
- Risk of corrupted pointers if not carefully tracked.
- Need to track and fix all internal addresses.

# 7. Segmentation: Enabling Relocation with Virtual Addresses

## 7.1 Motivation

- Goal: Move memory segments without breaking pointer references.
- Solution: Maintain separation of virtual and physical address spaces.

## 7.2 Segmentation Mechanism

### Virtual Address Space

- Memory addresses seen by process.
- Independent of physical RAM location.

### Segment Types

- Code segment.
- Data segment.
- Stack segment.
- Shared libraries (e.g., dynamically linked libraries).

### Segment Base Registers

- Each type of memory segment has a base register:
  - Code Base
  - Data Base
  - Stack Base
  - Auxiliary Base (e.g., libraries)

### Address Translation

- Virtual address + segment base register → physical address.
- Performed transparently by hardware.

## 7.3 Protection

- Segment also has a length or end address.
- Enforced by hardware.
- Access outside allowed bounds triggers a **segmentation fault**.

## 7.4 Benefits

- **Relocation**:
  - Move entire segments.
  - Only need to update segment base registers.
- **Isolation**:
  - Enforced by base + bound registers.
- **Performance**:
  - Hardware performs translation.
  - Fast and enforced at instruction level.

## 7.5 Segment-Based Memory Model Diagram

| Segment Type     | Virtual Address (v) | Base Register | Physical Address (p = v + base) |
|------------------|---------------------|---------------|-------------------------------|
| Code             | 0x100               | 0x300000      | 0x300100                      |
| Data             | 0x200               | 0x400000      | 0x400200                      |
| Stack            | 0x10                | 0x500000      | 0x500010                      |

# 8. Summary

This lecture provided a deep exploration of foundational memory management concepts in operating systems. It began by outlining the critical role of RAM and the challenges in managing it due to its limited nature. The lecture covered traditional memory management strategies including fixed and dynamic partitioning, explaining internal and external fragmentation with concrete examples. Allocation strategies (best fit, worst fit, first fit, next fit) were analyzed for performance and fragmentation trade-offs. Buffer pools were presented as a specialized solution for frequent, fixed-size allocations, while garbage collection and memory compaction were introduced as mechanisms to recover and optimize memory use.

Finally, the lecture introduced segmentation and base-register address translation as a precursor to virtual memory, enabling relocation, dynamic allocation, and protection mechanisms. These strategies allow moving toward more modern memory management approaches that will be covered in the next lecture.