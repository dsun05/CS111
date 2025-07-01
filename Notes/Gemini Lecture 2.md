# Operating System Abstractions and Services

## 1\. The Concept of Abstraction in Operating Systems

An **abstraction** in the context of an operating system (OS) is a simplified, higher-level representation of a complex, underlying hardware or software resource. The OS creates these abstract resources, which are easier for programmers and users to work with, by implementing them using the actual physical resources.

  * **Goal**: To bridge the gap between simple, powerful hardware resources and the more complex, powerful resources that applications need.
  * **Mechanism**: The OS adds a layer of software support on top of the physical hardware to create and manage these abstractions. When a program makes a system call, it is accessing these abstract services, not directly interacting with the physical hardware. The OS mediates between the application's high-level requests and the low-level physical operations.
  * **Implementation**: Abstractions are not magic; they are built using the real, physical components of the computer, such as RAM, CPU cores, and storage devices.

-----

### 1.1 Examples of OS Abstractions

  * **Process**: There is no such thing as a "process" in hardware; hardware only executes instructions. The OS builds the process abstraction on top of the CPU's instruction set and RAM, creating the illusion of separate, concurrent programs.
  * **File**: Storage devices like flash drives do not inherently understand files. They can only read, write, or erase fixed-size blocks of data (e.g., 4K or 16K). The OS takes on the job of organizing these blocks to create the file abstraction, which allows for storing and accessing variable-sized data collections.

-----

### 1.2 Reasons for Using Abstractions

Abstractions are fundamental to modern computing for several key reasons:

  * **Simplicity and Ease of Use**: They are simpler for programmers and users to understand and work with. A programmer can simply write 100 bytes to a file without needing to know that the underlying hardware can only write a 4K block.
  * **Hiding Complexity**: Abstractions encapsulate and hide the immense complexity of the underlying system.
      * **Hardware Diversity**: Programmers don't have to worry about the specific block sizes of different storage devices (e.g., 2K, 4K, 8K).
      * **Asynchronous Operations**: They hide the complexity of dealing with slow devices and asynchronous events like interrupts. For example, instead of a program constantly polling a disk to see if a write is complete, the OS manages this using interrupts, letting the program continue with other tasks.
      * **Resource Management**: On a modern computer with potentially hundreds of processes wanting to run simultaneously on a limited number of CPU cores (e.g., 300 processes on 8 cores), the OS handles the complex scheduling decisions.
  * **Eliminating Irrelevant Behavior**: They hide inconvenient hardware characteristics.
      * **Flash Memory**: A key example is flash memory. At the hardware level, a block on a flash drive can only be written to once. To change it, a whole sector (a large group of blocks) must be erased, which is a very slow operation. The OS provides an abstraction that hides this "erase-before-write" complexity, making the drive appear as if it can be written to like any other storage medium.
  * **Creating Convenient Behavior**: Abstractions can provide the illusion that a process has exclusive access to a shared resource.
      * **Network Interface**: A computer may have one network card, but dozens of programs (browser, email, games) may want to use it simultaneously. The OS provides an abstraction that gives each program the illusion that it has its own dedicated network connection, managing the underlying sharing of the single physical interface.
  * **Generalizing for Portability**: Abstractions create a unified model for different types of hardware, allowing applications to be written once and run on many different machines.
      * An application like Microsoft Word needs to run on computers with different processors, screens, keyboards, and storage devices.
      * Instead of writing custom versions for each hardware combination, developers write to an abstract keyboard, an abstract screen, etc. The OS is responsible for translating the operations on these abstract devices into the specific commands required by the actual hardware on a given machine.
      * **PDF (Portable Document Format)** is a common example of a unifying model, allowing a document to be printed correctly on virtually any modern printer.
      * **SCSI (Small Computer System Interface)** is a standard for storage devices that allows the OS to treat various block-oriented devices (disks, CDs) in a uniform way.

-----

## 2\. Types of OS Resources

The OS manages different types of resources, which can be categorized based on how they are used by multiple clients (e.g., processes).

### 2.1 Serially Reusable Resources

These are resources that multiple clients can use, but only one at a time. This is a form of **time multiplexing**, where clients take turns using the resource.

  * **Access Control**: The OS must enforce exclusive use, ensuring that only the current designated client can access the resource.
  * **Graceful Transitions**: When the resource is switched from one user to another, the transition must be "graceful."
  * **Examples**:
      * **Computer**: Printers, speakers.
      * **Real World**: Bathroom stalls.

#### 2.1.1 Graceful Transitions Explained

A **graceful transition** is a switch between users of a resource that completely hides the fact that someone else used it previously. This involves two key aspects:

1.  **Cleaning Up**: The OS must ensure that no data or state from the previous user is left behind. The resource should appear to be in a "like new" or clean state for the new user.
2.  **Restoring State**: Often, a resource is taken away from a process temporarily and will be given back later. In this case, the graceful transition must restore the resource to the *exact state* it was in when it was taken away, making it seem to the process as if it never lost the resource at all.

-----

### 2.2 Partitionable Resources

These are resources that can be divided into multiple, disjoint pieces, with each piece allocated to a different client. This is known as **spatial multiplexing**.

  * **Access Control**: This is crucial to enforce:
      * **Containment**: A client cannot access resources outside of its assigned partition.
      * **Privacy**: Other clients cannot access the resources *within* your partition.
  * **Graceful Transitions**: These are still necessary because partitions are typically not allocated permanently. A chunk of RAM might be used by Process A now, but will be given to Process B later. When this switch happens, a graceful transition is needed to clean and prepare the partition for the new process.
  * **Examples**:
      * **Computer**: RAM, disk drive storage.
      * **Real World**: Hotel rooms.

-----

### 2.3 Shareable Resources

These are resources that can be used by multiple clients concurrently without needing to wait or own a specific subset.

  * **Characteristics**: Often, these resources are effectively limitless or are non-state-changing (read-only).
  * **Graceful Transitions**: Usually not needed, because the resource either doesn't change state or isn't "reused" in a way that leaves behind user-specific data. It doesn't get "dirty."
  * **Benefit**: Shareable resources are highly efficient because they eliminate the overhead of managing turns or partitions and performing graceful transitions. It is a good design principle to maximize the use of shareable resources in complex systems.
  * **Examples**:
      * **Computer**: The execute-only code of the operating system itself. All 300 processes on a machine can share one copy of the OS code in memory.
      * **Real World**: The air in a lecture hall.

-----

### 2.4 Comparison of Resource Types

| Feature                | Serially Reusable         | Partitionable                   | Shareable                  |
| :--------------------- | :------------------------ | :------------------------------ | :------------------------- |
| **Multiplexing** | Time (one at a time)      | Space (divided into pieces)     | Concurrent (all at once)   |
| **Access Control** | Required for exclusive use| Required for containment/privacy| Not typically needed       |
| **Graceful Transition**| Required for user switches| Required for partition realloc. | Usually not required       |
| **Example** | Printer, Speakers         | RAM, Disk Space                 | OS Code, Air in a Room     |

-----

## 3\. Critical OS Abstraction Classes

There are three foundational classes of abstractions that most operating systems provide. Other, more complex abstractions are often built upon these.

### 3.1 Memory Abstractions

All programs require memory to store data, such as variables, allocated chunks, files, and messages. While all memory abstractions involve reading and writing, they are complicated by several factors.

  * **Complicating Factors**:
      * **Persistence**: Memory can be **persistent** (data survives power loss, e.g., flash drive) or **transient** (data is lost when power is off, e.g., RAM).
      * **Operation Size**: The size of data an application wants to work with (e.g., one byte) often differs from the fixed size the physical hardware works with (e.g., a 4K block).
      * **Latency**: The time it takes to complete an operation varies dramatically between memory types (e.g., RAM is extremely fast, disks are slow, tapes are very slow).
      * **Coherence and Atomicity**:
          * **Coherence**: Ensures that after a write operation is confirmed, all subsequent reads will see the new data.
          * **Atomicity**: Ensures that a multi-part operation (like writing an 8K record to a device with a 4K block size) happens as a single, indivisible unit. Either the entire 8K is written, or none of it is; there is no intermediate state visible to other processes.
  * **The OS's Role**: The OS must implement file systems and memory management techniques to handle these complexities. This includes using strategies like **wear-leveling** for flash drives (distributing erases evenly to prolong device life) and **garbage collection** to manage discarded data blocks.

-----

### 3.2 Interpreter Abstractions

An **interpreter** is any component, physical or abstract, that performs commands.

  * **Physical Interpreter**: The CPU is the fundamental physical interpreter.
  * **Abstract Interpreter**: The OS provides higher-level interpreter abstractions, with the most common being the **process**.

A basic interpreter has four main components:

1.  **Instruction Reference**: Points to the next command to be executed (e.g., a program counter).
2.  **Repertoire**: The set of all possible commands the interpreter can perform (e.g., the code of a program).
3.  **Environment Reference**: The current state on which instructions operate (e.g., a process's stack, heap, and registers).
4.  **Interrupts**: A mechanism to override the normal instruction flow to handle urgent events.

<!-- end list -->

  * **The OS's Role**: With hundreds of processes but only a few CPU cores, the OS must manage these abstractions. This requires:
      * **Schedulers**: To decide which process runs on which core and for how long.
      * **Memory Management**: To give each process the illusion that it has its own private memory, even though physical RAM is limited and shared.
      * **Access Control**: To protect a process's resources (like files) from other processes.

-----

### 3.3 Communication Abstractions

A communication link allows interpreters to exchange information, either on the same machine or across a network.

  * **Physical Implementation**: On a single machine, this is done by copying data in RAM. Between machines, it's done via physical cables and network interfaces.
  * **Key Differences from Memory Abstractions**:
      * **Asynchronous Nature**: Senders and receivers do not act in perfect lockstep. A receiver may have to wait for a sender, and a sent message is not guaranteed to be received immediately.
      * **Variable Performance**: Network performance can be highly variable and unpredictable.
      * **Reliability and Security**: The internet is not inherently reliable (messages can be lost) or secure.
  * **The OS's Role**: The OS must provide mechanisms to manage this complexity. This involves:
      * **Optimizing Copying**: Since copying data has a performance cost, the OS uses tricks to make it efficient.
      * **Implementing Network Protocols**: The OS includes complex protocol stacks (like TCP/IP) to handle message loss, retransmission, and security.

-----

## 4\. OS Services and Their Delivery

The primary job of an OS is to offer services to applications and users, typically through the abstractions it provides. These services can be delivered in different ways, operating at different layers of the software stack.

### 4.1 Layers of an Operating System

A modern OS is structured in layers, from the physical hardware at the bottom to the user applications at the top.

  * **Hardware**: The base layer, including the CPU, memory, and I/O devices. It provides a raw **Instruction Set Architecture (ISA)**.
  * **Operating System Kernel**: The core of the OS. It runs in **privileged mode** and has direct access to the hardware. It handles critical tasks that require privilege.
  * **Libraries**: Collections of pre-written code (e.g., libc) that applications can use. They often act as an intermediary between applications and the kernel's system calls.
  * **System Services**: Trusted helper programs that run outside the kernel (e.g., `login` process, `sendmail` daemon) to perform specific system-level tasks.
  * **Applications**: The programs that users interact with. They run in **non-privileged mode**.

-----

### 4.2 Service Delivery via Subroutines (Libraries)

One way to provide services is through simple subroutine calls to a library. This method is used for tasks that do not require special hardware privileges.

  * **Mechanism**: An application calls a function in a library just as if it were its own code. This involves pushing parameters onto the stack, jumping to the function's code, and returning.
  * **Advantages**:
      * **Speed**: Extremely fast, with very little performance overhead.
  * **Disadvantages**:
      * **No Privileges**: The library code runs with the same (non-privileged) permissions as the application.
      * **Same Address Space**: The library and application must reside in the same process address space.
  * **Example**: Calling a standard library function to concatenate two strings or calculate a sine value.

#### 4.2.1 Library Binding Options

The process of connecting an application's call to the actual library code is called **binding**. This can happen at different times.

1.  **Static Libraries**: The library code is physically copied into the application's executable file at **link time**.
      * **Result**: Each application has its own private copy of the library. This is very wasteful of both disk space and RAM, as the same code is duplicated many times.
2.  **Shared Libraries**: A single copy of the library exists on disk and a single copy is loaded into memory. All processes that use the library **share** this one copy.
      * **Result**: This is a shareable resource. It dramatically reduces memory and disk usage and speeds up program startup if the library is already in memory. Updates are also simplified; updating the one shared file updates it for all applications.
3.  **Dynamic Libraries (or Dynamically Loaded Libraries)**: The library is loaded into memory only when and if the application explicitly requests it at **run time**.
      * **Result**: This is useful for large applications like Microsoft Word that have many features the user might not use in a given session. It avoids loading code for unused features, saving memory.

-----

### 4.3 Service Delivery via System Calls

This is the primary method for an application to request a service that requires OS-level privileges.

  * **Mechanism**: A **system call** forces a controlled entry into the OS kernel. This process involves a mode switch from the application's non-privileged mode to the kernel's privileged mode. The requested service is then performed by code inside the kernel.
  * **Advantages**:
      * **Privilege**: Allows the use of privileged instructions and access to protected kernel data structures.
      * **Inter-Process Communication**: Enables secure communication between isolated processes, which is managed by the OS.
  * **Disadvantages**:
      * **Speed**: System calls are significantly slower (100x to 1,000x) than subroutine calls due to the overhead of the context switch between user and kernel mode.
  * **When to Use**: System calls are necessary for any operation involving:
      * I/O with peripheral devices (disks, networks, etc.).
      * Allocation of physical resources like memory.
      * Managing processes and ensuring their privacy and containment.
      * Handling interrupts.

-----

## 5\. OS Interfaces: API and ABI

To ensure that programs can be developed and used across many different systems, operating systems define stable, well-defined interfaces.

### 5.1 API (Application Program Interface)

The API is a **source-level** interface for programmers.

  * **Definition**: It specifies the set of functions, data types, constants, and macros that a programmer can use to access OS services.
  * **Purpose**: To provide **software portability**. A programmer can write a program to a specific API (e.g., the Windows API or the POSIX API for Linux), and that source code can then be recompiled to run on any machine that supports that API, regardless of the underlying hardware architecture.
  * **Audience**: Primarily for programmers.

-----

### 5.2 ABI (Application Binary Interface)

The ABI is a **binary-level** interface.

  * **Definition**: It specifies the low-level details needed for a compiled program to run, such as data formats, calling conventions (how parameters are passed to functions), and how to make a system call on a specific hardware architecture. It is the binding of an API to a specific hardware ISA.
  * **Purpose**: To provide **binary compatibility**. A program compiled for a specific ABI (e.g., for x86-64 Linux) will run on *any* computer that matches that hardware architecture and operating system without modification or recompilation.
  * **Audience**: Primarily for end-users, even though they are unaware of it. The ABI is what allows a user to download and run an executable file directly.

-----

### 5.3 Interface Stability and Side Effects

  * **Stability**: It is critical for OS vendors to keep their APIs and ABIs stable over time. If an interface changes with an OS update, existing applications that were written for the old interface will break, leading to a very poor user experience.
  * **Side Effects**: A **side effect** is an un-specified, non-obvious consequence of an action that is not part of the official interface definition. Programmers who rely on these undocumented behaviors are at risk, because the OS developers are free to change their internal implementation at any time, which may eliminate the side effect and break the program. Good software engineering practice is to avoid relying on side effects and to design systems that do not have them.

-----