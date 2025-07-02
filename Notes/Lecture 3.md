# 1. The Process: An Introduction

A process is a fundamental concept in operating systems, representing an active instance of a program being executed. It serves as the primary mechanism through which users can run code and perform tasks on a computer. Nearly everything a user initiates on a computer occurs within the context of a process. The operating system is responsible for creating, executing, controlling, and managing processes and their states.

### 1.1. What is a Process?

A process can be understood from several perspectives:

* **An Executing Program:** It is the active, running instance of a program. For example, when you open a web browser or a game, you are running a process for that application.
* **A Virtual Computer:** The operating system creates the illusion that each process has its own virtual computer, with complete control over the system's resources. In reality, the operating system remains in control and may prevent a process from performing certain actions.
* **An Object:** In computing, an object is an entity that possesses both **state** (data describing its properties) and **operations** (actions it can perform). A process is a central type of object managed by the operating system, crucial for the functioning of modern systems like Windows, macOS, and Linux.

# 2. Process State

The state of an object refers to the data that describes its current condition, distinguishing it from other objects of the same type. For a process, its state includes information like which system resources have been allocated to it. The state of an object is dynamic and changes over time as it executes.

### 2.1. The Nature of State

* **Representation:** The state of any object within a computer can be represented as a collection of bits.
* **Manipulation:** This bit representation is significant because it allows the operating system to save the state of an object (e.g., a process), temporarily remove the object, and later restore it perfectly by using the saved bits. This also enables the creation of identical copies of an object.
* **Complexity:** More complex objects generally have more complex states. Within a complex object like a process, the state is often partitioned into subsets for different purposes, such as scheduling information.

### 2.2. Elements of Process State

A process's state is a collection of various pieces of information, including:
* Scheduling priority.
* The program it is executing.
* Its memory space (address space), which includes pages holding its stack and other data.
* Open files and I/O operations.

Most of a process's state is managed by the operating system. An application typically must request the OS to make changes to its state, such as ending the process or modifying a file pointer. However, some elements of a process's state are directly controllable by the program itself.

# 3. The Address Space

A crucial component of a process's state is its **address space**, which is the set of all memory addresses that the process is legally permitted to use for instructions, data, and the stack. Any attempt by a process to access a memory address outside of its designated address space will result in an error.

* **Potential Size:** In modern 64-bit systems, the potential address space is vast ($2^{64}$ addresses), far exceeding the physical RAM available in any machine. The operating system must manage this discrepancy.
* **Uniqueness:** The OS also manages how multiple processes can each believe they have access to the same addresses (e.g., address 10,000) without conflict.

### 3.1. Programs vs. Processes

It is essential to distinguish between a program and a process.

| Feature | Program | Process |
| :--- | :--- | :--- |
| **Location** | On disk (e.g., flash drive). | In RAM (memory). |
| **State** | Inert, passive data. | Active, running, with a dynamic state. |
| **Nature** | A file containing code and data. | An executing instance of a program. |

### 3.2. Program Structure on Disk (ELF)

An executable program on disk, such as in the **Executable and Linkable Format (ELF)**, is structured with several key components.
1.  **ELF Header:** This initial section identifies the file as an executable program and provides critical information, such as the Instruction Set Architecture (ISA) it was compiled for (e.g., Intel vs. PowerPC). The OS checks the ISA before attempting to run the program.
2.  **Code Section:** This segment contains the compiled, machine-language instructions of the program.
3.  **Data Section:** This holds initialized data, such as predefined variables and their initial values.
4.  **Info Sections (e.g., Symbol Table):** These sections are not loaded into memory for execution but contain useful metadata. The **symbol table** maps variable and function names from the source code (e.g., `foo`, `main`) to their memory locations. It is used primarily by debuggers to allow programmers to inspect the program's state using the original names.

### 3.3. Process Layout in Memory

To run a program, the OS creates a process and maps the program's components into the process's address space in RAM. This memory layout typically includes several key segments.

#### 3.3.1. Memory Segments and Permissions

Different segments of a process's memory have different properties and access permissions.

| Segment | Permissions | Description |
| :--- | :--- | :--- |
| **Code** | Read-Only, Executable | Contains the machine instructions. It is not writable to prevent self-modifying code. |
| **Data (Heap)** | Read/Write | Holds dynamically allocated memory and global variables. Must be readable and writable. |
| **Stack** | Read/Write | Used for function calls, local variables, and parameters. Must be readable and writable but should not be executable to prevent security vulnerabilities. |

#### 3.3.2. Typical Linux Memory Layout

A common strategy in Linux is to organize the address space as follows:
* The **code segment** is placed at the lowest addresses. It is static and does not change in size.
* The **data segment (heap)** is placed directly after the code. It grows towards higher addresses (up) as memory is dynamically allocated.
* The **stack segment** is placed at the highest end of the address space. It grows towards lower addresses (down) as functions are called.

The operating system ensures that the heap and the stack do not grow into each other and collide.

# 4. Detailed Look at Process Segments

### 4.1. Code Segment

* **Creation:** The code segment originates from a load module created by a linkage editor, which combines various compiled object files (`.o` files) into a single executable.
* **Loading:** When a process is created, the OS copies the code from the program file on disk into the allocated memory region for the process. This memory is then marked as execute-only.
* **Sharing:** Since code segments are read-only, they can be safely shared among multiple processes running the same program. For instance, if four separate compilations are running simultaneously using GCC, all four processes can share a single copy of the GCC code in memory, saving space.

### 4.2. Data Segment (Heap)

* **Initialization:** The data segment is initialized with values defined in the program file. It also includes uninitialized space for dynamic allocation.
* **BSS Segment:** A "Block Started by Symbol" (BSS) segment contains uninitialized data that the OS is expected to zero out upon loading. This is an efficient way to prepare large data areas that will be filled in during runtime.
* **Privacy:** Unlike the code segment, the data segment is private to each process. Each of the four GCC processes would have its own private, non-shared data segment to store information about the specific program it is compiling.
* **Growth:** The size of the data segment is often unknown at the start and can grow dynamically as the program runs. In Linux, the `sbreak` system call is used to request more memory for the data segment.

### 4.3. Stack Segment

* **Function:** Modern stack-based languages rely on the stack for program execution. Each time a function is called, a new **stack frame** is pushed onto the stack. When the function returns, its frame is popped off.
* **Stack Frame Contents:** A stack frame holds:
    * Local variables defined within the function.
    * Parameters passed to the function.
    * Saved register values to be restored after the function returns.
    * The return value of the function.
* **CPU Support:** Modern CPUs have hardware features to make stack operations more efficient.
* **Size and Management:** The stack size is dynamic and depends on the program's execution path, especially with recursive function calls. The OS allocates an initial default size for the stack, which can be increased dynamically if needed.
* **Privacy:** Like the data segment, the stack is private to each process and must be read/write but not executable.

# 5. Libraries

### 5.1. Static Libraries

* **Integration:** Static libraries are linked directly into the program's code segment by the linkage editor. At runtime, they are simply part of the code and are loaded along with it.
* **Upgrading:** If a static library is updated, any program that uses it must be relinked to incorporate the new version.

### 5.2. Shared (Dynamic) Libraries

* **Loading:** Shared libraries are not part of the program's code segment on disk. When a process starts, the OS checks if the required shared libraries (e.g., `libc`) are already in memory.
* **Advantages:**
    * **Memory Savings:** If a popular library is already in RAM, the OS simply maps it into the new process's address space, avoiding redundant copies.
    * **Faster Startup:** The process starts faster because the library code doesn't need to be loaded from disk.
    * **Easier Upgrades:** Libraries can be updated without needing to relink the programs that use them.

# 6. The Process Descriptor

The address space contains much of a process's state, but not all of it. Other critical state information is stored in hardware registers and in a special OS data structure.

### 6.1. Hardware Registers

* **General-Purpose Registers:** Used for fast, temporary storage of data during computations.
* **Special-Purpose Registers:**
    * **Program Counter (PC):** Points to the memory address of the next instruction to be executed.
    * **Processor Status Word (PSW):** Contains status bits, such as whether the CPU is in privileged (supervisor) or normal (user) mode, and flags for arithmetic results (e.g., overflow, negative).
    * **Stack Pointer and Frame Pointer:** Registers that track the location of the current stack and the active stack frame.

### 6.2. OS-Managed Information

The OS also maintains other state information for each process, including:
* Open file descriptors.
* The current working directory.
* Accounting information, like total CPU time used.

### 6.3. The Process Control Block (PCB)

To organize all this information, the operating system uses a data structure for each process, known generically as a **process descriptor**. In Linux/Unix systems, this is called a **Process Control Block (PCB)**.

* **Function:** The PCB is the central repository for all information the OS needs to manage a process, including scheduling, security, and resource allocation.
* **Contents:** A PCB typically contains or points to:
    * Process ID (a unique number).
    * Process state (e.g., running, blocked).
    * The program counter and CPU register values.
    * A pointer to the process's address space structure.
    * Pointers to open file tables.
    * Scheduling and priority information.
    * Accounting information (CPU time, etc.).
* **Protection:** The PCB is managed exclusively by the OS. A process cannot directly read or modify its own PCB; it must make a system call to request changes.

# 7. Process Lifecycle Management

The OS performs a few fundamental operations on processes: creation, destruction, and execution.

### 7.1. Process Creation

Processes are always created by the operating system, though typically at the request of an existing process. This creates a parent-child relationship between processes.

To create a process, the OS must:
1.  Create and initialize a new process descriptor (PCB).
2.  Add the new PCB to the system's process table (a data structure tracking all active processes).
3.  Allocate an address space and associated memory for the code, data, and stack.
4.  Load the program code into the code segment.
5.  Initialize the stack.
6.  Set the initial values for the PC, PSW, and other registers.

There are two main models for process creation:

#### 7.1.1. Creation from Scratch (Windows Model)

* **Method:** In this model, an empty or "blank" process is created, and all its necessary attributes (the program to run, memory sizes, etc.) are specified as parameters to the creation call.
* **System Call:** Windows uses the `CreateProcess` system call for this purpose. This call is highly flexible, with many optional parameters to configure the new process precisely.

#### 7.1.2. Forking (Linux/Unix Model)

* **Method:** This model, called **forking**, creates a new process by cloning an existing one. The new process (the child) is an almost exact duplicate of the creating process (the parent).
* **System Call: `fork()`:** The `fork()` system call is used to create the clone. It takes no parameters. The parent and child share the same code segment but get separate copies of the stack.
* **System Call: `exec()`:** Typically, the goal is to run a *different* program in the new process. To achieve this, the child process immediately calls the `exec()` system call. `exec()` replaces the child's current memory image (code, data, and stack) with that of a new program.

This `fork()`-then-`exec()` pattern is the standard way to launch new programs in Linux, for example, when a command shell or a window manager starts a new application.

* **Optimization (Copy-on-Write):** A naive `fork()` would be inefficient if it had to copy a very large data segment, only for `exec()` to immediately discard it. Modern systems use an optimization called **copy-on-write**, where the data segment is initially shared. A private copy is only made if and when one of the processes attempts to write to the data.

### 7.2. Process Termination

Processes can be terminated for several reasons:
* The program completes its execution normally.
* The user manually terminates it (e.g., with Ctrl-C).
* The operating system terminates it due to an unrecoverable error or by request.

When a process is terminated, the OS must reclaim all of its resources:
* Release its memory (RAM).
* Close any open files.
* Release any locks it holds.
* Notify other related processes (like its parent) of its termination.
* Deallocate its PCB and remove it from the process table.

# 8. Process Execution and Exceptions

### 8.1. Loading and Limited Direct Execution (LDE)

To run a process, the OS must **load** it onto a CPU core. This involves initializing the core's hardware registers with the process's state (its PC, PSW, stack pointer, etc.). If the process has run before, its saved state is restored.

Once loaded, the process runs in a mode called **Limited Direct Execution (LDE)**.
* **Direct Execution:** The CPU executes the process's instructions directly, one after another, without OS intervention for each one. This is crucial for performance.
* **Limited:** The execution is "limited" because the OS retains ultimate control. The hardware will automatically transfer control back to the OS under certain conditions (exceptions).

The goal of a well-designed OS is to maximize the time spent in LDE, as this corresponds to running user code and achieving high performance.

### 8.2. Exceptions

An **exception** is any event that causes the CPU to stop executing the current stream of instructions and transfer control to the OS.

* **Synchronous Exceptions (Traps):** These are caused by the instruction currently being executed.
    * **Faults:** Unintentional errors like arithmetic overflow or attempting to access an illegal memory address (segmentation fault).
    * **System Calls:** Intentional requests for OS services, triggered by a special `trap` instruction.
* **Asynchronous Exceptions (Interrupts):** These are caused by events external to the CPU and independent of the current instruction.
    * **I/O Completion:** A device (like a disk) signals that it has finished an operation.
    * **Timer Interrupt:** A programmable clock generates an interrupt, allowing the OS to regain control periodically.
    * **External Signals:** Events like a power failure or a user pressing Ctrl-C.

### 8.3. Trap Handling for System Calls

System calls are the primary mechanism for a process to request services from the OS. They are implemented using a hardware-supported trap mechanism.

1.  **User Process:** The application prepares for the system call by placing the system call number and any parameters into predefined registers (e.g., R0, R1). It then executes a special `trap` instruction.
2.  **Hardware Trap:** Because the `trap` instruction is privileged, executing it in user mode causes a hardware exception. The CPU hardware automatically:
    * Switches from user mode to privileged **supervisor mode**.
    * Saves the current process's PC and PSW.
    * Looks up the address of the OS's trap handler in a special hardware structure called the **trap vector table**.
    * Jumps to that address.
3.  **OS First-Level Handler:** This generic OS code runs. It saves the rest of the user process's registers. It then examines the system call number (from R0) to determine which specific service was requested.
4.  **OS Second-Level Handler (The Gate):** Based on the system call number, the first-level handler dispatches to the specific OS routine (the "trap gate") that implements the requested service (e.g., the code to open a file).
5.  **Execution and Return:** The system call code executes. Upon completion, the process is reversed: the second-level handler returns to the first, which restores all the user process's registers, switches the CPU back to user mode, and returns control to the user process, which continues its execution from where it left off.

#### 8.3.1. User and Supervisor Stacks

To keep OS data isolated and secure from user processes, each process typically has two stacks:
* **User Stack:** Used during normal execution in user mode.
* **Supervisor Stack:** Used by the OS when it is handling a trap or interrupt on behalf of the process. When a trap occurs, the system switches to using the supervisor stack, ensuring that all OS-related function calls and data are kept separate from the user's address space.

# 9. Blocked Processes

A process can be in one of two fundamental states regarding its readiness to run:
* **Ready (or Runnable):** The process is able to run, but may be waiting for a CPU core to become available.
* **Blocked:** The process is waiting for some event to occur and cannot run, even if a CPU core is free.

### 9.1. Reasons for Blocking

A process becomes blocked when it must wait for a slow operation to complete. Common reasons include:
* Waiting for I/O (e.g., reading from a disk).
* Waiting for a network message to arrive.
* Waiting for another process in inter-process communication.

### 9.2. Managing Blocked State

* **State Information:** The OS keeps track of whether a process is blocked or ready in its PCB.
* **Scheduling:** The scheduler, which selects which process to run next, will never choose a process that is marked as blocked.
* **Unblocking:** The component that caused the block (e.g., the disk I/O resource manager) is responsible for unblocking the process once the event it was waiting for has occurred. For I/O, this typically happens when the device generates an interrupt to signal completion. When a process is unblocked, the scheduler is often invoked to reconsider which process should be running, as the newly ready process might have a higher priority.

---
