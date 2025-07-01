# Lecture 3: Processes, Execution, and State

## 1. What Is a Process?

A process is a fundamental abstraction provided by the operating system. It is the active entity that allows code to be run on a computer.

* **An Interpreter:** A process is a specific and crucial type of interpreter used by the operating system.
* **An Executing Instance of a Program:** A program is a static file on a disk (e.g., Microsoft Word, a web browser). A process is the active, running instance of that program in memory.
* **A Virtual Private Computer:** The operating system creates the illusion that each process has the entire computer to itself, with full access to all resources and control over the machine. In reality, the OS remains in control and enforces limitations.
* **An OS Object:** A process is a type of object in computing, characterized by its state (properties and data) and its operations (things it can do). While other OS objects exist (like files), processes are central to the behavior of modern operating systems.

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

### State Management

The management of a process's state is a shared responsibility.

* **OS-Managed State:** Most of a process's state is managed by the OS. This includes the process descriptor, open file lists, and allocated memory segments. A user program must make a request to the operating system (a system call) to access or modify the state of these OS-managed objects.
* **Process-Managed State:** The process itself is responsible for the contents of its own private memory, specifically its stack and data (heap) segments. The process can read from and write to these areas directly. This means a program must be careful, as it has the ability to corrupt its own stack or heap.

## 3. The Process Address Space

A process's address space is the complete set of memory addresses that the process is legally allowed to access. If an address is not in its address space, the process cannot use it. Modern operating systems create the illusion that each process has a vast address space (e.g., up to $2^{64}$ addresses on a 64-bit machine), even though the physical RAM is much smaller.

### Program vs. Process

There is a critical distinction between a program on disk and a running process in memory.

A **program** is a passive, executable file stored on a disk, such as in the **Executable and Linkable Format (ELF)**. This file is the output of a linkage editor and contains all the necessary components to run, organized into distinct sections.

  * **ELF Header**: This identifies the file as executable and specifies the target **Instruction Set Architecture (ISA)**, ensuring the OS doesn't try to run an incompatible program.
  * **Code and Data Sections**: These contain the compiled machine instructions and initialized global variables.
  * **Symbol Table**: This is a metadata section that is not loaded into memory for execution. It maps source code names for variables and functions to their addresses, which is essential for debugging tools.

A **process** is an active instance of a program running in memory. The OS creates a process by loading the program's code and data into a dedicated **address space** in RAM.

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

4.  **Libraries**
    * **Static Libraries:** The library code is copied directly into the program's code segment by the linkage editor before execution. To update the library, the program must be re-linked.
    * **Shared Libraries:** A single copy of the library exists in memory and is shared by all processes that use it. The OS loads the library and maps it into the address space of each process. This reduces memory usage, speeds up program loading, and makes library upgrades easier.

## 4. Process State and Descriptors

While the address space contains much of a process's state, other critical pieces of information are stored elsewhere and managed by the OS.

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

## 5. Handling Processes

The OS is responsible for the entire lifecycle of a process: creation, execution, and destruction.

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

### Process Destruction (Termination)

When a process terminates (either by finishing its work, being killed by a user, or by the OS), the OS must perform a cleanup.

**OS Steps for Destruction:**
1.  **Reclaim Resources:** Take back all resources the process was holding, including memory, locks, and access to hardware devices.
2.  **Inform Others:** Notify any relevant processes, such as the parent process, that the child has terminated.
3.  **Remove Descriptor:** Remove the process's PCB from the process table and reclaim the memory used by the PCB itself.

## 6. Running Processes and Execution

### Limited Direct Execution (LDE)

To maximize performance, operating systems use a model called **limited direct execution**.

* **Direct Execution:** Most of the time, the process's instructions are executed directly by the CPU core without any OS intervention. The CPU fetches and executes instructions one after another at full hardware speed.
* **Limited:** This execution is limited because the process runs in **user mode**, which prevents it from executing privileged instructions (like those for I/O or modifying system state). If a process needs to perform a privileged operation, it must transfer control to the OS.

### Exceptions, Traps, and System Calls

An **exception** is any event that interrupts the normal flow of instruction execution.

* **Asynchronous Exceptions:** Occur unpredictably from the process's point of view (e.g., a user hitting Ctrl-C, a segmentation fault from a bad pointer, a power failure signal).
* **Traps:** A trap is a type of exception generated intentionally by software to transfer control to the OS. This is the mechanism used for **system calls**.

**System Call Mechanism:**
1.  **Setup:** The user process prepares for the system call by placing the system call number and any arguments into specific registers, as defined by the system's conventions.
2.  **Trap Instruction:** The process executes a special `trap` instruction. This is a privileged instruction.
3.  **Mode Switch:** Because a user-mode process tried to execute a privileged instruction, the CPU hardware automatically traps into the OS. It switches the CPU from user mode to **supervisor mode** and begins executing an OS trap handler.
4.  **OS Handler:**
    * The OS saves the state of the user process (PC, registers).
    * It examines the registers to identify the requested system call number.
    * It uses a **system call dispatch table** to look up and jump to the specific OS code that implements the requested service.
5.  **Service Execution:** The OS executes the requested service (e.g., opening a file, allocating memory).
6.  **Return:** Once the service is complete, the OS restores the user process's state and executes a special privileged instruction to return to user mode, resuming the process's execution at the instruction immediately following the `trap`.

### Trap Handling and Stacks

To handle traps securely, the OS uses two separate stacks for each process:
* **User-Mode Stack:** The normal stack used by the process during its execution in user mode.
* **Supervisor-Mode Stack:** A separate, private stack that is only used by the OS when it is handling a trap or interrupt on behalf of that process. This prevents the OS from writing sensitive kernel data onto the user's stack, which the user process could potentially read later.

### Interrupts

An **interrupt** is a signal sent to the CPU from a hardware device (e.g., a network card, a disk controller, or a system timer). It is similar to a trap but is triggered by hardware instead of software. When an interrupt occurs, the CPU stops what it's doing and transfers control to an OS **interrupt handler** to service the device. This is the mechanism used for asynchronous I/O and for time-sharing between processes.

## 7. Managing Process State

### Blocked Processes

A key piece of a process's state is whether it is runnable or blocked.

* **Blocked State:** A process is **blocked** if it is waiting for some event to occur and cannot make any useful progress until it does. Common reasons for blocking include:
    * Waiting for an I/O operation to complete (e.g., reading from a disk).
    * Waiting for a resource to become available.
    * Waiting for another process in inter-process communication.
* **Scheduling:** The process's state (blocked or ready) is stored in its PCB. The OS **scheduler** will only consider running processes that are in the "ready" state. It will not attempt to run a blocked process, which avoids wasting CPU time.

### Blocking and Unblocking

* A process is typically blocked by a part of the OS called a **resource manager**. When a process requests a resource that is not immediately available, the resource manager changes the process's state to "blocked" and calls the scheduler to run a different process.
* When the event the process was waiting for occurs (e.g., the disk read completes), the resource manager is notified (typically via an interrupt). It then changes the process's state from "blocked" to "ready" and notifies the scheduler that a new process is now available to run.
