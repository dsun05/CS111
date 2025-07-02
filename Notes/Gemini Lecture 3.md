# Lecture 3: Processes, Execution, and State

## 1. What Is a Process?

A process is a fundamental abstraction provided by the operating system. It is the active entity that allows code to be run on a computer.

* **An Interpreter:** A process is a specific and crucial type of interpreter used by the operating system.
* **An Executing Instance of a Program:** A program is a static file on a disk (e.g., Microsoft Word, a web browser). A process is the active, running instance of that program in memory.
* **A Virtual Private Computer:** The operating system creates the illusion that each process has the entire computer to itself, with full access to all resources and control over the machine. In reality, the OS remains in control and enforces limitations.
* **An OS Object:** A process is a type of object in computing, characterized by its state (properties and data) and its operations (things it can do). While other OS objects exist (like files), processes are central to the behavior of modern operating systems.

***

## 2. The Concept of State

State refers to the data that describes the current condition of a persistent object, distinguishing it from other objects of the same type.

* **Definition:** All persistent objects (objects that exist for a duration of time) have a state. This state characterizes the object's current condition and distinguishes it from other objects of the same type. The state of an object is not static and changes as the object is used and operations are performed on it.
* **Representation:** The state of any object in a computer can be represented as a set of bits. This is significant because it means the OS can save the state (by copying the bits) and restore it later, effectively recreating the object's exact condition. This is fundamental for operations like stopping and restarting a process.
* **State Subsets:** For complex objects like processes, the total state can be divided into subsets used for specific purposes, such as a "scheduling state" which contains information relevant only to scheduling the process.

### Examples of OS Object State

| Object Type | State Example |
| --- | --- |
| **Process** | Scheduling priority. |
| **Process** | The list of memory pages allocated to it. |
| **File** | The current pointer indicating the next read location. |
| **I/O Operation**| The completion condition (e.g., has a message been sent?). |

***

### State Management

The management of a process's state is a shared responsibility.

* **OS-Managed State:** Most of a process's state is managed by the OS. This includes the process descriptor, open file lists, and allocated memory segments. A user program must make a request to the operating system (a system call) to access or modify the state of these OS-managed objects.
* **Process-Managed State:** The process itself is responsible for the contents of its own private memory, specifically its stack and data (heap) segments. The process can read from and write to these areas directly. This means a program must be careful, as it has the ability to corrupt its own stack or heap.

***

## 3. The Process Address Space

A process's address space is the complete set of memory addresses that the process is legally allowed to access. If an address is not in its address space, the process cannot use it. Modern operating systems create the illusion that each process has a vast address space (e.g., up to $2^{64}$ addresses on a 64-bit machine), even though the physical RAM is much smaller.

***

### Program vs. Process

There is a critical distinction between a program on disk and a running process in memory.

A **program** is a passive, executable file stored on a disk, such as in the **Executable and Linkable Format (ELF)**. This file is the output of a linkage editor and contains all the necessary components to run, organized into distinct sections.

  * **ELF Header**: This identifies the file as executable and specifies the target **Instruction Set Architecture (ISA)**, ensuring the OS doesn't try to run an incompatible program.
  * **Code and Data Sections**: These contain the compiled machine instructions and initialized global variables.
  * **Symbol Table**: This is a metadata section that is not loaded into memory for execution. It maps source code names for variables and functions to their addresses, which is essential for debugging tools.

A **process** is an active instance of a program running in memory. The OS creates a process by loading the program's code and data into a dedicated **address space** in RAM.

***

### Address Space Layout

The operating system has a strategy for arranging the different memory segments a process needs within its address space. Different segments have different requirements (e.g., permissions).

* **Linux Process Layout:** A common strategy in Linux systems is to:
    * Place the **code segment** at the lowest addresses. The size of the code is static.
    * Place the **data segment (heap)** immediately after the code. The data segment *grows up* toward higher addresses as more memory is dynamically allocated.
    * Place the **stack segment** at the highest end of the address space. The stack *grows down* toward lower addresses as functions are called.
    * The OS ensures that the data segment and stack segment are not allowed to meet and overlap.
 
```
+------------------+  <-- Highest Address
|       Stack      |
|  (grows down) V  |
|                  |
| (available memory) |
|                  |
|   ^ (grows up)   |
|       Heap       |
+------------------+
|   Initialized    |
|       Data       |
+------------------+
|       Code       |
+------------------+  <-- Lowest Address
```

***

### Address Space Segments

1.  **Code Segments**
    * Originate from the load module on disk, which is the output of a linkage editor.
    * The OS loads the code from the program file into RAM for execution.
    * Permissions are typically **read-only** and **execute-only**. Self-modifying code is not permitted in modern systems.
    * Code segments can be **sharable**. If multiple processes are running the same program (e.g., multiple instances of the GCC compiler), they can all share a single copy of the code segment in memory, saving space.

2.  **Data Segments (Heap)**
    * Used for global variables and dynamically allocated memory (the heap).
    * Initial contents are copied from the load module.
    * **BSS (Block Started by Symbol)** segments are a part of the data segment that are uninitialized in the program file and are simply zeroed-out by the OS when the process starts.
    * Permissions are **read/write**.
    * Data segments are **process-private**. Each process has its own separate data segment that no other process can access.
    * The size can be grown or shrunk dynamically by the program, for instance, by using the `sbrk` system call in Linux.

3.  **Stack Segment**
    * Modern programming languages are stack-based. The stack is used to manage function calls.
    * Each function call allocates a new **stack frame**, which stores:
        * Local variables for the function.
        * Parameters passed to the function.
        * Saved register values to be restored after the function returns.
    * The stack's size is dynamic, growing as functions are called and shrinking as they return.
    * The OS can dynamically extend the stack segment if the process needs more space.
    * Permissions are **read/write** and it is **process-private**.
    * For security reasons, the stack is typically **not executable**.

***

4.  **Libraries**
    * **Static Libraries:** The library code is copied directly into the program's code segment by the linkage editor before execution. To update the library, the program must be re-linked.
    * **Shared Libraries:** A single copy of the library exists in memory and is shared by all processes that use it. The OS loads the library and maps it into the address space of each process. This reduces memory usage, speeds up program loading, and makes library upgrades easier.

***

## 4. Process State and Descriptors

While the address space contains much of a process's state, other critical pieces of information are stored elsewhere and managed by the OS.

***

### Other Process State Components

* **Registers:** Hardware registers on the CPU core are essential.
    * **General Purpose Registers:** Used for temporary calculations.
    * **Program Counter (PC):** Points to the address of the next instruction to execute.
    * **Processor Status Word (PSW):** Contains status bits, such as whether the CPU is in supervisor or user mode, or if the last arithmetic operation resulted in an overflow.
    * **Stack Pointer (SP):** Points to the location of the stack.
    * **Frame Pointer (FP):** Points to the current stack frame.
* **OS Resources:** The OS tracks resources allocated to the process.
    * Open files.
    * Current working directory.
    * Locks.
* **OS-Internal Information:**
    * Accounting information, like the total CPU time the process has used.

***

### Process Descriptors (Process Control Block - PCB)

The OS uses a data structure to keep track of all this information for every process. This is generically called a **process descriptor**.

* **Linux PCB:** In Linux and other Unix-like systems, this structure is called the **Process Control Block (PCB)**.
* **Contents:** The PCB stores all information relevant to a process, including:
    * A unique Process ID (PID).
    * The current state of the process (e.g., running, blocked).
    * Pointers to its address space information.
    * Scheduling priority.
    * Saved values of the CPU registers (when not running).
    * Information about open files and other I/O resources.
    * Accounting information.
* **Management:** The PCB is entirely managed by the OS. The process cannot directly access or modify its own PCB. It resides in a **process table** (or a similar data structure like a list or tree) that the OS uses to organize all currently active processes.

***

## 5. Handling Processes

The OS is responsible for the entire lifecycle of a process: creation, execution, and destruction.

***

### Process Creation

Processes are always created by the OS, typically at the request of another existing process. This creates a parent-child relationship.

**OS Steps for Creation:**
1.  Create a new process descriptor (PCB).
2.  Add the new PCB to the process table.
3.  Create a new address space for the process.
4.  Allocate memory for the process's code, data, and stack segments.
5.  Load the program's code and initialized data from the file on disk into the allocated memory.
6.  Initialize the stack.
7.  Set up initial register values in the PCB, including the Program Counter (to the program's entry point) and the Stack Pointer.

***

**Two Models for Process Creation:**

1.  **Windows: Starting With a "Blank" Process**
    * Uses the `CreateProcess()` system call.
    * This is a highly flexible call with many parameters that specify exactly how the new process should be configured.
    * At a minimum, the call must specify the name of the program file to be run.
    * The OS creates a new process from scratch according to these specifications.

2.  **Unix/Linux: Forking**
    * This model uses two distinct system calls: `fork()` and `exec()`.
    * **`fork()`:** This system call takes no parameters. It creates a new child process by creating an almost exact clone of the calling parent process.
        * The child gets its own unique PID but inherits almost everything else.
        * It shares the parent's code segment.
        * It gets its own private copy of the parent's stack.
        * The data segment is handled with a **copy-on-write** optimization. The child initially shares the parent's data pages (marked as read-only). If either process tries to write to the data, a private copy of that specific page is made for that process. This avoids a costly full copy, especially since the child often calls `exec()` immediately.
    * **`exec()`:** After a `fork()`, the child process typically calls `exec()` immediately. This system call replaces the child's entire current process image (code, data, and stack) with a new program. The OS gets rid of the old segments and loads the new program specified in the `exec()` call.

***

### Process Destruction (Termination)

When a process terminates (either by finishing its work, being killed by a user, or by the OS), the OS must perform a cleanup.

**OS Steps for Destruction:**
1.  **Reclaim Resources:** Take back all resources the process was holding, including memory, locks, and access to hardware devices.
2.  **Inform Others:** Notify any relevant processes, such as the parent process, that the child has terminated.
3.  **Remove Descriptor:** Remove the process's PCB from the process table and reclaim the memory used by the PCB itself.

***

## 6. Running Processes and Execution

### Loading Processes

When a process is ready to run, the OS must load it onto a CPU core. This involves initializing the core's hardware with the process's state, which is either its initial state if it's a new process or its previously saved state if it was paused.

The steps to load a process are:

* **Load Registers**: The OS copies the saved values of the general-purpose registers, stack pointer, and frame pointer from the process's PCB into the hardware registers on the CPU core.
* **Set Up Memory**: The OS configures the core's memory management unit to use the process's address space.
* **Load the Program Counter**: The final step is to load the program counter (PC) with the address of the next instruction the process should execute.

Once the PC is loaded, the CPU immediately begins fetching and running instructions directly, which marks the beginning of Limited Direct Execution.

***

### Limited Direct Execution (LDE)
To maximize performance, operating systems use a model called **Limited Direct Execution (LDE)**. This approach balances speed with security by allowing a process's code to run directly on the CPU while providing mechanisms for the OS to intervene and remain in control.

***

### The "Direct Execution" Component
For the majority of its runtime, a process's non-privileged instructions are executed **directly** by the CPU core. The OS loads the process's state onto the core, sets the program counter, and then lets the hardware run the code at full speed without any OS intervention for each instruction. This is the key to achieving good system performance, as it avoids the overhead of constantly involving the OS. The goal is to keep the application running its own code as much as possible.

***

### The "Limited" Component
The execution is **limited** in two crucial ways to ensure the OS stays in charge.

#### 1. Limitation via User Mode and Traps
A process runs in **user mode**, which restricts it from performing privileged operations that could affect system stability, such as accessing I/O devices directly or modifying memory outside its address space. If a process needs to perform a privileged operation, it must request it from the OS via a **system call**. This is done by executing a special `trap` instruction, which immediately stops the process and transfers control to the OS to handle the request safely.

#### 2. Limitation via Timer Interrupts
A process might run for a long time without ever making a system call (e.g., a long scientific computation or an infinite loop). To prevent a single process from monopolizing the CPU, the OS needs a way to forcibly regain control. This is accomplished using a **timer interrupt**.

Before dispatching a process to run, the OS sets a hardware timer to generate an interrupt after a specific interval (a "time slice"). When the timer goes off, the interrupt forces the CPU to stop the current process—no matter what it's doing—and transfer control to an OS interrupt handler. Once in control, the OS can perform a **context switch**: it can save the state of the current process and load the state of a different one, thus allowing for fair time-sharing among all running processes.

***

### Exceptions, Traps, and System Calls
An **exception** is any event that interrupts the normal flow of a program's instruction execution. The operating system and hardware provide mechanisms to handle these events, which can be broadly categorized as either synchronous or asynchronous.

* **Synchronous Exceptions**: These exceptions occur as a direct and predictable result of an instruction being executed. A well-written program can often anticipate these conditions. Examples include arithmetic overflow, trying to read past the end of a file, or data conversion errors.

* **Asynchronous Exceptions**: These exceptions occur at unpredictable times, often triggered by events external to the code's direct logic. Examples include a segmentation fault from accessing a bad memory address, a user aborting the program (e.g., `Ctrl-C`), or a power failure signal from the hardware.

***

### Traps
While a process runs in user mode, it cannot perform privileged operations. When it needs a service that requires such privileges—like writing to a file or allocating memory—it must ask the OS to do it. The process does this by executing a special `trap` instruction.

A **trap** is a specific type of exception that is intentionally generated by software to switch from user mode to supervisor mode and request a service from the operating system. It's the underlying mechanism for all **system calls**.

***

### The System Call Mechanism
The process of using a trap for a system call is a highly structured, procedure divided into distinct hardware and software portions.:

***
### Hardware Portion of Trap Handling
When a process executes a `trap` instruction, the CPU hardware initiates a sequence of non-interruptible actions:
* The hardware uses the type of exception (in this case, a system call trap) as an index into a special, hardware-defined **trap vector table**. This table contains the starting addresses for different OS exception handlers.
* It saves the current state of the user process, specifically the **Program Counter (PC)** and the **Processor Status Word (PSW)**, by pushing them onto a stack.
* It loads a new Processor Status Word, which officially switches the CPU into **supervisor mode**.
* Finally, it loads the Program Counter with the address of the OS's **first-level trap handler** from the vector table, effectively transferring control to the OS.

***
### Software Portion of Trap Handling
Once the hardware has transferred control, the OS software takes over:

* The **first-level handler** runs. Its first job is to save the rest of the user process's state by pushing all other general-purpose registers onto the stack.
* It then gathers information about the system call by reading the registers where the user process placed the system call number and arguments.
* Using the system call number, it looks up the appropriate service routine in a **system call dispatch table** and calls the corresponding **second-level handler**.
* The **second-level handler** is the OS code that actually implements the requested service (e.g., the code to open a file or allocate memory). It can run a significant amount of OS code to complete its task.

Upon completion, the process is reversed: the second-level handler returns, the first-level handler restores all the saved registers from the stack, and a privileged "return from trap" instruction restores the PC and PSW, seamlessly returning the process to user mode.

***
### The Supervisor-Mode Stack
For this entire process to be secure, the OS does not use the process's user-mode stack. Instead, it maintains a separate, private **supervisor-mode stack** for each process. All the state information (PC, PSW, registers) is pushed onto this private stack when handling a trap. This prevents the OS from leaking sensitive kernel data or addresses onto the user's stack, which the user process could potentially read or corrupt later.

***

### Asynchronous Events and Interrupts
Some events, like waiting for a disk read to complete, are worth waiting for, while others demand immediate attention without the process actively checking for them. These are **asynchronous events**. The primary mechanism computers use to handle them is the **interrupt**.

An **interrupt** is a signal sent from a hardware device to the CPU to report an event that requires the operating system's attention. It is similar to a trap but is triggered asynchronously by hardware, not by a software instruction.

***
### Purpose of Interrupts
Interrupts are essential for enabling a modern computer to multitask efficiently. They serve two primary purposes:

* **Handling Asynchronous I/O**: When a process requests data from a slow device like a disk, it would be inefficient for the CPU to wait. Instead, the OS initiates the request and can switch to another process. When the device has finished its operation, it sends an interrupt to the CPU. This interrupt signals the OS to handle the completed I/O and make the data available to the original process.
* **Enabling Time-Sharing**: To prevent any single process from monopolizing the CPU, the OS uses a hardware **timer**. This timer is set to generate an interrupt after a specific interval, or "time slice". When the interrupt occurs, the OS regains control and can decide to switch to a different process, ensuring all processes get a fair share of CPU time.

***

### Interrupt Handling
When an interrupt occurs, the CPU immediately stops executing the current process and transfers control to an OS **interrupt handler**. This process is similar to trap handling, where the state of the current process is saved, and the OS takes over in supervisor mode to service the event. Once the event has been handled, the OS can either resume the interrupted process or switch to a different one.

***

### User-Mode Signal Handling
The OS can send various **signals** to a process to inform it of events like a user pressing `Ctrl-C` or a segmentation fault. The process can control how it reacts to these signals in one of three ways:

* **Ignore the signal**, effectively pretending it never happened.
* **Provide a custom handler**, which is a specific function within the process's code designed to run when the signal is received.
* **Accept the default action**, which for most critical signals is to terminate the process.

This mechanism gives a program a structured way to react to otherwise unpredictable events.

***
### Distinction from Interrupts and Traps
While related, signals are distinct from lower-level hardware and software exceptions like interrupts and traps.

| Feature | **Trap** | **Interrupt** | **Signal** |
| :--- | :--- | :--- | :--- |
| **Origin** | **Software**: Generated intentionally by a process executing a special instruction. | **Hardware**: Generated asynchronously by a hardware device (e.g., timer, disk). | **Operating System**: A software abstraction generated by the OS to notify a process of an event. |
| **Purpose** | To make a **system call** and request a service from the OS. | To notify the OS of a hardware event that needs servicing. | To notify a **user-mode process** of an event. |
| **Handler** | An OS kernel trap handler. | An OS kernel interrupt handler. | A user-defined signal handler or a default OS action. |

***

## 7. Managing Process State
The management of a process's state is a shared responsibility between the operating system and the process itself. The OS secures and manages system-level resources, while the process manages its own internal data.

***
### OS-Managed State
The OS is responsible for managing all the state information that affects the system as a whole. This information is typically stored in the **Process Control Block (PCB)** and is inaccessible to user-mode code. The OS manages:

* **Resource Allocation**: Which memory segments, open files, and hardware devices have been allocated to the process.
* **Scheduling Information**: The process's priority and whether it is ready to run or blocked.
* **The Supervisor-Mode Stack**: The private stack used by the OS when handling traps or interrupts for the process.

A process cannot change this state directly; it must make a system call to request a change from the OS.

***
### Process-Managed State
The process itself is directly responsible for managing the contents of its own private memory segments, specifically its **stack** and its **data (heap)** areas. The process has read/write access to this memory and is free to manipulate it. This freedom means a buggy or careless program can corrupt its own stack or heap, often leading to a crash.

***

### Blocked Processes

A key piece of a process's state is whether it is runnable or blocked. A process is **blocked** when it cannot make any useful progress until an external event occurs, such as waiting for an I/O operation to complete or for a requested resource to become available. The operating system manages this state to ensure CPU time is not wasted on processes that are unable to run.

***
### Blocking a Process
When a process requests a resource that is currently unavailable, the part of the OS managing that resource (the **resource manager**) will take the following steps:
* It changes the process's scheduling state in its PCB to "blocked".
* It effectively tells the OS scheduler not to choose this process to run.
* It may then yield the CPU to allow the scheduler to run a different, ready process.

A process can also request to be blocked itself via a system call, but it must be sure that another process or the OS will eventually unblock it.

***
### Unblocking a Process
The resource manager that blocked the process is responsible for unblocking it. When the required resource becomes available:
* The resource manager changes the process's scheduling state from "blocked" back to "ready".
* It then notifies the scheduler that a change has occurred, making the newly-ready process eligible to be run again.
