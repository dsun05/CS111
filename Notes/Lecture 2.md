# 1. Introduction to Operating System Abstractions and Services

This lecture discusses the foundational concepts in operating systems, focusing on:
- Abstractions and services.
- How operating systems create and manage these abstractions.
- Types of abstractions (memory, processor, communication).
- Resource management strategies.
- Interfaces (APIs, ABIs).
- Service delivery mechanisms like system calls and libraries.

---

# 2. Abstractions in Operating Systems

## 2.1. Definition of Abstraction
An abstraction in operating systems is a higher-level concept built on top of hardware resources, made usable to applications and users by software.

- Created using software to hide hardware complexity.
- Designed for easier use and consistency across devices.
- Not inherently supported by hardware.

### Key Examples:
| Abstraction | Underlying Hardware | Description |
|-------------|----------------------|-------------|
| Process     | CPU, RAM             | Represents a running program with its context. |
| File        | Block device (e.g., flash drive) | Logical representation of stored data. |
| Network socket | Network card     | Provides a communication endpoint. |

## 2.2. Why Use Abstractions?
- Simplify interaction with complex hardware.
- Allow developers to target a common interface.
- Improve portability and reusability.
- Isolate users/programmers from hardware inconsistencies.
- Enable performance optimizations.

## 2.3. Surpassing Hardware Limitations
Examples:
- Flash drives can only operate in blocks (e.g., 4K). The OS allows byte-level file operations.
- CPUs only execute instructions — no notion of "processes". OS builds scheduling and context management on top.

## 2.4. Consistency and Performance
- Abstractions prevent developers from needing to adjust for different hardware speeds or block sizes.
- OS can optimize underlying implementations for performance aids like buffering or asynchronous I/O (using interrupts).

---

# 3. Types of Computer Resources and OS Resource Management

The OS manages various types of hardware resources by abstracting them into standardized models, which can be:

## 3.1. Serially Reusable Resources
Used by one user/process at a time, shared over time (temporal multiplexing).
- Examples: Printer, Speakers, Bathroom stalls.
- Requires access control and graceful transitions to maintain isolation.
  - Must reset state when switching users.

### Characteristics:
- Access Control: Only one authorized process can use the resource at a time.
- Graceful Transition: Resource state cleanup and/or restoration.


## 3.2. Partitionable Resources
Divided into chunks across users (spatial multiplexing).
- Examples: RAM, disk space, hotel rooms.

### Key Requirements:
- Disjoint allocation: Prevent overlaps between users.
- Access control.
- Graceful transitions if reused by another process (reset data/state).

## 3.3. Shareable Resources
Multiple clients use the full resource simultaneously.
- Examples: Operating system code (read-only), air in a room.

### Characteristics:
- Maximize efficiency and accessibility.
- Minimal overhead and management.
- If resource is read-only (e.g., OS code), access is naturally safe.

---

# 4. Core Operating System Abstractions

## 4.1. Memory Abstractions

### Characteristics:
| Attribute      | Description |
|----------------|-------------|
| Persistence    | Whether memory retains data across power cycles. |
| Granularity    | Word/block/byte access size supported. |
| Latency        | Time delay to complete memory operations. |
| Coherence      | Ensuring latest writes are visible to readers. |
| Atomicity      | All parts of an operation complete or none do. |

### Memory Types:
| Memory Type      | Persistent | Typical Use |
|------------------|------------|-------------|
| RAM              | No         | Program execution, heap/stack. |
| Flash drives     | Yes        | Non-volatile file storage. |
| Hard drives      | Yes        | File storage, OS, data. |
| Database records | Mixed      | Permanent or cache use. |
| Files            | Yes        | Long-term data store. |

### Memory Complications:
- Performance differences.
- Abstracted file allows variable sizes (ex: 100 bytes) despite underlying block size (ex: 4K).
- OS hides physical details like block size and erase sector size (especially acute in flash drives).

## 4.2. Interpreter Abstractions (Processes and Threads)

### Interpreter Components:
| Component             | Realization          |
|-----------------------|----------------------|
| Instruction reference | Program counter      |
| Repertoire            | Machine code         |
| Environment reference | Stack, heap          |
| Interrupt mechanism   | Handled via OS       |

- Physical interpreters: CPU cores.
- Abstract interpreters: Processes, threads.
- Each interpreter (e.g., process) must be context-isolated and have dedicated register/state info.

### Challenges:
- Mapping 300 process states to 8 CPU cores (context switching).
- Register scarcity.
- Graceful interruption (and state saving/resumption).

## 4.3. Communication Abstractions

### Fundamental Challenges:
- Processes are isolated: must deliberately share/communicate.
- Can occur locally (between two processes on same machine) or remotely (across network).

### Characteristics of Communication Abstractions:
| Attribute          | Memory Abstraction | Communication Abstraction |
|--------------------|--------------------|----------------------------|
| Access             | Always guaranteed   | Sender must actually send |
| Synchronicity      | Mostly synchronous  | Often asynchronous        |
| Medium             | Local memory copy   | Network message/protocol  |
| Performance        | Stable              | Highly variable           |

### Implementation Tools:
- IPC (Inter-process communication): pipes, message queues.
- Network stacks (e.g., TCP/IP).
- Abstracted so that differences between local and remote communication are minimized.

---

# 5. Service Delivery Mechanisms

## 5.1. Layers in Modern Operating Systems

Top to bottom hierarchy:
1. **Users**: Interact with applications.
2. **Applications**: Rely on libraries/OS for resources.
3. **Libraries**: Provide frequently used routines, often wrap system calls.
4. **System Calls**: Interface to kernel for privileged operations.
5. **Kernel**: Manages hardware and core OS services.
6. **Hardware**: Actual CPU, memory, devices.

## 5.2. Libraries

### Types:
- **Static Libraries**:
  - Linked at compile time.
  - Code included in each load module (bigger files, redundancy).
- **Shared Libraries**:
  - Referenced dynamically at link/load time.
  - Only one memory instance, shared across processes.
- **Dynamically Loaded Libraries**:
  - Loaded at runtime when needed.
  - Example: Microsoft Word loading spell checker only upon use.

### Trade-offs:

| Library Type       | Pros                                         | Cons                                     |
|--------------------|----------------------------------------------|------------------------------------------|
| Static             | Simpler deployment, version control          | Larger disk/RAM usage, no central updates|
| Shared             | RAM efficient, updates propagate             | Harder debugging, no per-app versioning  |
| Dynamic            | Memory saving, load only when necessary      | Runtime overhead, complexity             |

### Library Binding Time:
| Type      | Binding Time   |
|-----------|----------------|
| Static    | Compile/Link   |
| Shared    | Load time      |
| Dynamic   | Runtime        |

## 5.3. System Calls

- Provide access to kernel-level services (I/O, memory allocation, scheduling).
- Enter privileged mode upon execution.
- Slower than library calls (100–1000x), but necessary for privileged operations.

### Typical Uses:
- File descriptors: `open()`, `write()`
- Memory: `mmap()`, `sbrk()`
- Process management: `fork()`, `exec()`, `kill()`
- Network: `socket()`, `connect()`, `recv()`

## 5.4. Service Delegation and Modularity

### System Daemons / Server Processes:
- Run trusted but unprivileged processes for certain services.
- Example: `login`, `sendmail`, NFS server.

### Kernel Plugins:
- Device drivers.
- File systems.
- Added as-needed per hardware.

---

# 6. Interfaces and Compatibility

## 6.1. Application Programming Interface (API)
- Source code level interface.
- Language-specific (e.g., C API for Linux).
- Abstracts system services (e.g., reading a file).
- Promotes software portability.

### Software Portability Matrix:

| Platform | Compile Needed | Same Binary Usable |
|----------|----------------|--------------------|
| Same OS + ISA | No        | Yes                |
| Same OS, Diff ISA | Yes   | No                 |
| Diff OS       | Yes        | Usually No         |

## 6.2. Application Binary Interface (ABI)
- Machine-level implementation of API.
- Defines binary-level calling conventions, data structures, system call mechanisms.
- Enables compatibility across systems of the same OS + ISA combination.

## 6.3. Importance of Interface Stability

- Developers and users expect programs built for one version to continue working on newer versions.
- Breaking the API/ABI compatibility leads to mass application failures.
- Especially critical for widely deployed software with commercial users.

---

# 7. Side Effects & System Stability

## 7.1. Side Effect Definition
An unintended but observable behavior from system operations, not specified in the interface.

## 7.2. Risks:
- Future OS updates may remove side effects, breaking dependent applications.
- Violates abstraction separation.
- Leads to non-portable, brittle code.

## 7.3. Historical Example:
- Game developers accessing display buffers directly for performance — broke on OS update.

## 7.4. Best Practices:
- Avoid depending on side effects.
- Treat interfaces as contracts.
- Stick to specified public APIs/ABIs.

---

# 8. Summary

This lecture explored operating system responsibilities in providing abstractions and services. It detailed how OS abstractions conceal hardware complexities through interfaces that are easier to manage and deliver performance and portability. Abstractions like processes, files, and memory are essential to coherent system operation. Resource types were discussed, along with strategies like access control and graceful state transitions. The lecture emphasized service delivery through libraries and system calls, contrasting static, shared, and dynamic approaches. Interfaces such as APIs (for source-level portability) and ABIs (for binary compatibility) were defined, along with the importance of their stability and the dangers posed by hidden side effects. The content provides a comprehensive foundation for understanding system design decisions and constraints in modern operating systems.