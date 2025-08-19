## **CS 111: Operating System Principles - Lecture 15 Notes**

### **1.0 Introduction to Virtual Machines (VMs)**

### **1.1 Defining "Virtual" in Computer Science**
In computer science, the term **"virtual"** signifies that something is an illusion designed to appear real. It provides an abstraction that behaves like a real entity, but its underlying implementation is different.
*   A well-known example is **virtual memory**. It gives a process the illusion of having a vast, private memory space, when in reality, the physical RAM is limited and shared among many processes.

***

### **1.2 What is a Virtual Machine?**
Following the definition of "virtual," a **Virtual Machine (VM)** is not a real, physical machine. It is a software-based emulation of a computer system.
*   **A Software Illusion of a Real Machine:** A VM acts as if it were a complete, standalone computer with its own hardware components (CPU, RAM, storage, network interface), but it runs as a program on top of a real, physical "host" computer. It allows you to run applications and even entire operating systems within a self-contained environment.
*   **Common Abbreviation: VM:** The term is commonly abbreviated as **VM**. This can sometimes be confusing, as **`VM`** is also used to abbreviate **virtual memory**. The correct meaning is usually clear from the context.

***

### **1.3 The Core Concept: Abstraction**
Like many concepts in operating systems, a virtual machine is a powerful **abstraction**.
*   **Running on a Real, Physical Computer:** All computation must ultimately happen on real hardware. A VM leverages the physical resources of a host computer to execute the instructions of the software running inside it. The VM software's job is to manage this mapping between the virtual resources and the real ones.
*   **Making One Computer Look Like Many:** Virtualization allows a single powerful server to be partitioned into multiple, isolated virtual machines. Each VM can run its own operating system and applications, appearing to its users as a completely separate computer.
*   **Example: Running a Linux VM on a Windows Host:** A common use case is running an operating system different from the host's. For example, you can have a Windows machine as your primary computer but run a Linux operating system inside a VM. This allows you to run Linux-specific applications without needing a separate physical machine or rebooting into a different OS.

***

### **1.4 Graphical Representation of Virtualization**
The relationship between real and virtual hardware can be visualized as a layered system.

*   **Real Hardware Components:** At the bottom layer is the actual physical server. This is a tangible computer you can touch, containing a real CPU, real RAM, and real peripheral devices like network cards and hard drives.
*   **Virtual Server Components:** Running on top of this real hardware is a **virtual server**. This virtual server is a software construct that has its own virtual components: a virtual CPU, virtual RAM, and virtual peripherals.
*   **Mapping Virtual Hardware to Real Hardware:** The virtualization software creates the illusion of the virtual hardware by mapping its operations to the real hardware.
    *   An instruction for the **virtual CPU** is executed on the **real CPU**.
    *   Data for the **virtual RAM** is stored in the **real RAM**.
    *   The **virtual disk** stores its data on the **real disk**.

```
      +------------------------------------------------+
      |      Virtual Server (Software Illusion)        |
      |                                                |
      |   [Virtual CPU]  [Virtual RAM]  [Virtual Disk] |
      +------------------+--------------+--------------+
                         |              |
                         | Maps to...   |
                         V              V
      +------------------+--------------+--------------+
      |      Real Server (Physical Hardware)           |
      |                                                |
      |     [Real CPU]     [Real RAM]     [Real Disk]  |
      +------------------------------------------------+
```

***

### **2.0 Motivations for Using Virtual Machines**

### **2.1 Fault Isolation**
VMs provide a powerful boundary for containing failures.
*   **Traditional OS Crash vs. Virtual OS Crash:** In a traditional system, if the operating system crashes, the entire machine goes down, taking all running applications with it.
*   **Impact on Host vs. Other VMs:** If an operating system running inside a VM (a "guest" OS) crashes, it only takes down that specific VM. The underlying host OS and any other VMs running on the same physical machine are unaffected.
*   **Benefits for OS Development:** This isolation is a massive benefit for OS developers. In the past, testing new OS code was risky; a crash required a full system reboot, making debugging slow and difficult. With VMs, a developer can test an OS inside a virtual machine. If it crashes, they can easily analyze the state from the host and reboot just the VM, not the entire physical machine.
*   **Isolating Hardware Faults:** Some software bugs in device drivers can send commands that physically damage hardware. By interacting with a *virtual* device, such faulty commands can be contained. Damaging a virtual device has no effect on the underlying physical hardware.

***

### **2.2 Enhanced Security**
VMs create a stronger security boundary than the separation between processes on a single OS.
*   **Limitations of Process Abstraction:** An operating system provides isolation between processes. However, all processes on that OS still share some common resources, like the file system or inter-process communication (IPC) channels. A vulnerability in the OS or a misconfiguration could allow one process to affect another.
*   **How VMs Strengthen Isolation:** Each VM is a completely separate environment. A process in VM A cannot directly access the file system of VM B, because they are distinct virtual file systems. This makes it much harder for an attacker who compromises one VM to reach and damage applications or data in other VMs on the same physical host.

***

### **2.3 Running Different Operating Systems**
VMs are an elegant solution for running software designed for different operating systems on a single machine.
*   **The Problem: Incompatible Executables (APIs/ABIs):** A Linux executable cannot run on Windows (and vice-versa) because they are compiled for different Application Programming Interfaces (APIs) and Application Binary Interfaces (ABIs). They expect different system calls and library functions.
*   **Alternatives: Dual Booting, Separate Machines:** Before VMs were common, the solutions were to have two separate physical machines or to set up a "dual boot" system where you had to shut down and reboot the entire computer to switch between Windows and Linux.
*   **The VM Solution: Running Linux on Windows:** With virtualization, you can keep your host machine running Windows while simultaneously running a full Linux OS inside a VM. You can then run your Linux executable on the Linux VM seamlessly.

***

### **2.4 Controlled Resource Sharing**
VMs provide a more robust mechanism for partitioning and guaranteeing hardware resources.
*   **Difficulty of Guaranteeing Resource Allocation to Processes:** While an OS has mechanisms to schedule CPU time and manage memory, it is very difficult to strictly guarantee that "Process A gets exactly 20% of the CPU and 2 GB of RAM." The dynamic nature of the system makes such precise control hard.
*   **Ease of Allocating Resources to VMs:** It is much easier to configure a VM to have a specific allocation of resources (e.g., 2 virtual CPUs and 4 GB of RAM). The virtualization layer can enforce these boundaries much more strictly than an OS can for individual processes.
*   **Importance for Cloud Computing:** This capability is a cornerstone of **cloud computing**. Cloud providers can rent out a VM with a guaranteed set of resources to a customer, ensuring that the customer's performance is not affected by other customers (known as "tenants") running on the same physical hardware.

***

### **3.0 How Virtual Machines Work**

### **3.1 Foundational Principles**
To achieve good performance, virtualization relies on two key principles.
*   **Assumption: Same Instruction Set Architecture (ISA):** It is easiest and most performant if the virtual machine and the real machine use the same ISA (e.g., running an x86 VM on an x86 host). Emulating a different ISA (e.g., running an ARM VM on an x86 host) is possible but is much trickier and significantly slower.
*   **Strategy: Limited Direct Execution:** The goal is to run as much code from the VM directly on the host CPU as possible. Most user-level instructions (arithmetic, loads, stores, branches) are safe and can be executed at native hardware speed without any intervention. Intervention is only needed for privileged operations.

***

### **3.2 The Hypervisor (Virtual Machine Monitor - VMM)**
The core piece of software that enables virtualization is the hypervisor.
*   **Definition: The Controller/OS for VMs:** The **Hypervisor**, also known as the **Virtual Machine Monitor (VMM)**, is a layer of software that runs on the real hardware and manages one or more virtual machines. It is effectively a specialized operating system for operating systems.
*   **Role in Handling Traps and Privileged Operations:** Whenever code inside a VM (either an application or the guest OS itself) attempts to perform a privileged operation that could affect the host or other VMs, the hardware will **trap**. This trap is caught by the hypervisor, which then decides how to handle the operation safely. After handling it, the hypervisor returns control to the VM.

***

### **3.3 The Virtualization Architecture**

#### **3.3.1 The Old Architecture (Privileged Kernel)**
In a traditional computing architecture, the OS Kernel is the only software that runs in the CPU's privileged mode, giving it full control over the hardware.
*   _Visual Aid: Diagram of traditional OS architecture._
```
+-------------------------------------------------+
|          (User and System) Applications         |  <-- Non-Privileged Mode
+-------------------------------------------------+
|        OS Services / Middleware / Libraries     |
|-------------------------------------------------|
|            Application Binary Interface         |
+=================================================+
|            Operating System Kernel              |  <-- Privileged Mode
+-------------------------------------------------+
|                     Hardware (ISA)              |
+-------------------------------------------------+
```
***
#### **3.3.2 The New Architecture with a VMM**
Virtualization introduces a new layer—the hypervisor—and changes what runs in privileged mode.
*   **The VMM runs in privileged mode:** The hypervisor is the only component that gets to run in the CPU's highest privilege level. It has ultimate control over the hardware.
*   **Guest OS kernels run in non-privileged mode:** The operating systems running inside the VMs (the "guest" OSs) are "deprivileged." They run in a lower-privilege CPU mode, just like user applications. If they try to execute a privileged instruction, the CPU will generate a trap to the VMM.
*   _Visual Aid: Diagram of VMM architecture._
```
+-------------------------------------------------+
|          (User and System) Applications         |  <-- Non-Privileged Mode
+-------------------------------------------------+
|        OS Services / Middleware / Libraries     |
|-------------------------------------------------|
|            Application Binary Interface         |
+-------------------------------------------------+
|            Operating System Kernel (Guest)      |  <-- ALSO Non-Privileged!
+=================================================+
|                 The VMM / Hypervisor            |  <-- Privileged Mode
+-------------------------------------------------+
|                     Hardware (ISA)              |
+-------------------------------------------------+
```
***
#### **3.3.3 A More Complex Case: Multiple VMs on One Host**
The VMM architecture allows a single set of physical hardware to host multiple, completely different operating systems simultaneously.
*   _Visual Aid: Diagram showing multiple VMs (A, B, C) on a single VMM._
```
+----------------------+----------------------+----------------------+
| VM A (e.g., Linux)   | VM B (e.g., Windows) | VM C (e.g., macOS)   |
| [App 1] [App 2]      | [App 3] [App 4]      | [App 5] [App 6]      |
|----------------------|----------------------|----------------------|
| OS Kernel A          | OS Kernel B          | OS Kernel C          |
+====================================================================+
|                              The VMM                               |
+--------------------------------------------------------------------+
|                         One Set of Hardware                        |
|             (CPU, RAM, Disk, Network Interface, etc.)              |
+--------------------------------------------------------------------+
```
***

### **3.4 System Call Execution in a VM**
The process of handling a system call inside a VM illustrates the interaction between the guest OS and the VMM.
1.  **Application Issues a System Call:** An application running inside a VM executes a `trap` instruction to make a system call (e.g., to open a file).
2.  **Trap is Caught by the VMM:** Because the entire VM (including its OS) is running in non-privileged mode, this instruction traps to the only entity running in privileged mode: the **VMM**.
3.  **VMM Invokes Guest OS's Trap Handler:** The VMM knows which VM the trap came from. It also knows where that VM's OS has set up its trap handler table. The VMM simulates the trap event and passes control to the guest OS's trap handler, which now begins to execute (still in non-privileged mode).
4.  **Guest OS Attempts a Privileged Instruction:** As the guest OS handles the system call, it will eventually need to perform a truly privileged operation, such as sending a command to a disk controller. It issues this privileged instruction as if it had control of the hardware.
5.  **Instruction Traps Back to the VMM:** The CPU again prevents the guest OS from executing the privileged instruction and traps to the VMM.
6.  **The VMM Executes the Instruction:** The VMM validates the request, performs the necessary operation on the real hardware on behalf of the guest OS, and then returns control back to the guest OS to continue its work.

***

### **3.5 The Problem of Control and Isolation**
This architecture creates a fundamental challenge:
*   **Guest OS "Thinks" It's in Control:** The guest OS is designed to believe it has full control over the hardware, including managing page tables and device registers. We want to run unmodified operating systems whenever possible.
*   **VMM Must Enforce Isolation:** The VMM is actually in control and must enforce isolation between VMs. However, the VMM does not understand the internal state of the guest OS. For example, it doesn't know what processes the guest is running or how the guest has structured its page tables. The VMM must provide the illusion of control to the guest while secretly maintaining real control.

***

### **3.6 Potential Issues in Enforcing Isolation**
The fact that the guest OS runs in a non-privileged mode creates a significant security problem *inside* the VM itself.
*   **The Core Problem:** The guest OS (`Kernel A`) is supposed to protect its own internal data structures from the applications it runs (`App 1`, `App 2`). In a normal system, it does this using privileged instructions to manage memory protection.
*   **The Threat:** Since `Kernel A` is now running in non-privileged mode, what stops a buggy or malicious `App 1` from directly writing over the memory belonging to `Kernel A`? For example, `App 1` could corrupt the process control block for `App 2`, causing it to crash.
*   **The VMM's Dilemma:** One might think the VMM should handle this. However, the VMM has no understanding of the guest OS's internal abstractions. It doesn't know what a "process control block" is or which memory pages belong to the guest kernel versus a user application.
    *   The entity that **understands** the necessary protection boundaries (the guest OS) lacks the **power** (privilege) to enforce them.
    *   The entity with the **power** (the VMM) lacks the **knowledge** to know what to protect.

This issue is the primary motivation for **memory virtualization**, which provides a mechanism for the VMM to enforce these protections on behalf of the guest OS.

*   _Visual Aid: The interface the VMM must help protect._
```
         VM A (Running in Non-Privileged Mode)
+----------------------------------------------------+
|          [App 1]                 [App 2]           |
+----------------------------------------------------+
|  <---- HOW DO WE ENFORCE THIS INTERFACE? ---->     |
|  App 1 must not be able to corrupt Kernel A's data.|
+====================================================+
|              Operating System Kernel A             |
|          (e.g., process tables, page tables)       |
+----------------------------------------------------+
```

***

### **4.0 Memory Virtualization**

### **4.1 The Challenge: Sharing Physical RAM Securely**
The VMM must share the machine's single physical RAM among multiple VMs, each of which believes it has its own private, physical memory space. This must be done without allowing one VM to access the memory of another.

***

### **4.2 Adding a New Layer of Indirection**
The solution is to add another level of address translation. Traditional systems translate virtual addresses to physical addresses. Virtualized systems add a third type: machine addresses.

***

### **4.3 Three Tiers of Memory Addresses**
*   **Virtual Addresses:** Used by applications inside a VM. These are translated by the guest OS.
*   **Physical Addresses:** This is what the guest OS *thinks* are the real RAM addresses. However, in a virtualized environment, they are just another intermediate step.
*   **Machine Addresses:** These are the actual, real hardware addresses in the physical RAM chips. These are managed exclusively by the VMM.

*   _Visual Aid: Diagram illustrating the three address types._
```
  Application Address Space
+-----------------------------+
|      Virtual Address        |
+-----------------------------+
             |
             | Guest OS translates...
             V
+-----------------------------+
|      "Physical" Address     |  <-- (Not actually physical!)
+-----------------------------+
             |
             | VMM translates...
             V
+-----------------------------+
|       Machine Address       |  <-- (The real RAM address)
+-----------------------------+
```

***

### **4.4 Example: Handling a TLB Miss**
This two-level translation process becomes apparent when handling a TLB miss. The following example breaks down the complex flow of control.

#### **Part 1: The Initial TLB Miss and Trap to VMM**
1.  **Initial State:**
    *   `App 1` is running in **UNPRIVILEGED MODE**.
    *   The Guest OS, `Operating System Kernel A`, is also in **UNPRIVILEGED MODE**.
    *   The `VMM` (Hypervisor) runs in **PRIVILEGED MODE**.
2.  **Memory Access:** `App 1` attempts to access a virtual address `X`.
3.  **TLB Miss:** The hardware's Memory Management Unit (MMU) checks the Translation Lookaside Buffer (TLB) for a mapping for `X`. It is not found, resulting in a **TLB Miss**.
4.  **First Trap:** A TLB miss is a hardware fault that causes a **trap**. Because the CPU is in unprivileged mode, control is immediately transferred to the only software running in privileged mode: the **VMM**.

*   _Visual Aid: Part 1 - The first trap._
```
     RUNNING UNPRIVILEGED
+--------------------------------+
|         App 1                  | --(issues virtual address X)--> TLB MISS
|--------------------------------|
| Operating System Kernel A      |
+================================+
|          The VMM               | <--(catches the trap)
+--------------------------------+     RUNNING PRIVILEGED
|    [ TLB ]    [ RAM ]          |
+--------------------------------+
```

#### **Part 2: The Second Trap and Final Resolution**
5.  **VMM Invokes Guest OS:** The `VMM` catches the trap but doesn't know how to translate virtual address `X`. Only `Kernel A` knows about `App 1`'s page tables. The `VMM` invokes `Kernel A`'s registered TLB miss handler, passing control *up* to it. `Kernel A` is now running (still in unprivileged mode).
6.  **Guest OS Looks Up Translation:** `Kernel A` accesses `App 1`'s page table in memory and finds the translation from virtual address `X` to guest "physical" address `Y`.
7.  **Guest OS Attempts Privileged Instruction:** To make future accesses faster, `Kernel A` tries to install the `X -> Y` mapping into the TLB. The instruction to write to the TLB is **privileged**.
8.  **Second Trap:** Because `Kernel A` is running in unprivileged mode, its attempt to execute a privileged instruction causes a **second trap**, back down to the `VMM`.
9.  **VMM Performs Final Translation:** The `VMM` catches this second trap. It now knows that `Kernel A` wants to map `X` to `Y`. The `VMM` looks at its own internal page tables, which map guest "physical" addresses to real machine addresses. It translates `Y` to the actual machine address `Z`.
10. **VMM Updates TLB:** The `VMM`, running in privileged mode, writes the final, correct mapping (`X -> Z`) into the hardware TLB.
11. **Return and Resume:** The `VMM` returns control, the trap is handled, and execution unwinds back to `App 1`. `App 1` re-issues the instruction for virtual address `X`. This time, the translation is in the TLB, and the memory access succeeds instantly.

*   _Visual Aid: Part 2 - The second trap and resolution._
```
     RUNNING UNPRIVILEGED
+--------------------------------+
|         App 1                  | <--(eventually resumes)
|--------------------------------|
| Operating System Kernel A      | ----(tries to write to TLB)----> CAUSES 2ND TRAP
|  (looks up X -> Y)             |
+================================+
|          The VMM               |
|  (translates Y -> Z,           |
|   installs X -> Z in TLB)      | <--(catches 2nd trap)
+--------------------------------+     RUNNING PRIVILEGED
|    [ TLB ]    [ RAM ]          |
+--------------------------------+
```

***

### **4.5 Complexities and Implications**
This multi-layered approach introduces significant overhead.
*   **Page Faults:** Handling a full page fault (where the data must be read from disk) is even more complex, involving multiple traps back and forth between the guest OS and the VMM to manage disk I/O and memory allocation.
*   **Increased Overhead:** Every TLB miss now involves multiple context switches (App -> VMM -> Guest OS -> VMM -> App). These traps and the execution of additional system code in the VMM are expensive.
*   **Extra Paging Data Structures:** The VMM must maintain its own set of page tables (often called "shadow page tables") to map guest "physical" addresses to machine addresses, consuming additional memory.
*   **Inherent Performance Penalties:** Due to this overhead, running software inside a VM will inherently be slower than running it on bare metal, though modern hardware and software techniques have greatly reduced this penalty.

***

### **5.0 Improving VM Performance**

### **5.1 Hardware-Assisted Virtualization**
To mitigate performance penalties, CPU manufacturers have added special features to their hardware.
*   **Special CPU Features:** Technologies like **Intel VT-x** and **AMD-V** provide hardware support for virtualization. For example, they can handle the two-level address translation (guest-virtual -> guest-physical -> machine) directly in the MMU, which dramatically reduces the need for the VMM to trap and emulate memory management operations.

***

### **5.2 Paravirtualization**
Another approach is to modify the guest operating system to make it cooperate with the hypervisor.
*   **Modifying the Guest OS:** **Paravirtualization** involves changing the guest OS so that it is "aware" it is running inside a VM. Instead of trying to execute privileged instructions that will trap, it makes direct calls (called **"hypercalls"**) to the hypervisor to request services.
*   **How Awareness Helps:** This cooperative approach can be more efficient than the trap-and-emulate model because it avoids the overhead of a CPU trap. The guest OS and hypervisor communicate through a well-defined API, streamlining many operations.

***

### **6.0 Virtual Machines and Cloud Computing**

### **6.1 Defining the Cloud Computing Environment**
Cloud computing is a model for delivering on-demand computing services over the internet. These environments are built on a massive scale.
*   **Sharing Hardware Among Many Customers:** The core idea is multi-tenancy, where a single provider's hardware resources are shared among many different customers.
*   **Massive Scale:** Cloud providers operate data centers that are essentially warehouses filled with tens of thousands of servers, tightly packed into racks.
*   **High-Speed Networking and Remote Access:** All these machines are interconnected with high-speed internal networks and have massive connections to the internet, allowing customers to access their computing resources remotely.

***

### **6.2 The Business Case for VMs in the Cloud**
VMs are the fundamental technology that makes the cloud business model viable.
*   **Maximizing Hardware Utilization and Profit:** A cloud provider's goal is to maximize the number of paying customers they can support on a fixed amount of hardware. More customers equals more profit.
*   **Handling Fluctuating Customer Workloads:** Most customers do not use their maximum required resources all the time. They have peak needs and periods of low activity. By using VMs, a provider can "over-subscribe" a physical machine, meaning they place multiple VMs on it, knowing that it's unlikely all of them will demand their peak resources at the same time. The unused capacity of one customer's VM can be used by another.
*   **Providing Strong Isolation:** VMs ensure that these co-located customers cannot interfere with each other. The hypervisor enforces strict security and resource boundaries between them.

***

### **6.3 The VM Placement Problem**
A major operational challenge for a cloud provider is deciding which VMs to place on which physical machines.
*   **Efficiently Packing VMs onto Physical Machines:** This is a complex optimization problem. You have thousands of physical machines (the "bins") and tens of thousands of VMs (the "items"), each with different resource requirements (CPU, RAM, network). The goal is to find the most efficient packing.
*   **Bin Packing Algorithms:** This problem is a multi-dimensional version of the classic **Bin Packing** problem from computer science.
*   **The NP-Hard Challenge:** The bin packing problem is **NP-hard**, meaning there is no known efficient algorithm to find the absolute perfect, optimal solution, especially with many resource dimensions.
*   **Using Estimation Techniques:** Because finding the optimal solution is computationally infeasible, cloud providers use sophisticated **estimation techniques and heuristics** to find a "good enough" placement quickly.

***

### **6.4 Dynamic Resource Management**
Customer workloads are not static; they change over time.
*   **Re-adjusting VM placement:** The initial placement of VMs might become inefficient as some customers get busier and others become idle. The cloud controller must constantly monitor resource usage and readjust.
*   **Migrating Live VMs:** To rebalance load, cloud systems can perform **live migration**, moving a running VM from one physical host to another with no downtime for the application running inside it.
*   **Managing long-running jobs:** For services like large web servers that run continuously, the cloud provider can dynamically add or remove VMs to handle traffic spikes, and migrate them across the data center to optimize resource usage without interrupting the service.

***

### **7.0 Conclusion**

### **7.1 VMs as a Critical Technology in Modern Computing**
Virtual machines are no longer an exotic niche; they are a foundational technology used everywhere from developer laptops to the world's largest cloud data centers.

***

### **7.2 Key Implementation Challenge: The Illusion of Control**
The central technical challenge in building a VM system is providing the guest OS with the convincing illusion that it has complete control over a set of hardware resources, while the hypervisor secretly retains ultimate control and enforces isolation.

***

### **7.3 The Constant Trade-off: Functionality vs. Performance**
Like all system software, VMs embody a trade-off. They provide immense flexibility, security, and portability, but these benefits come at the cost of some performance overhead compared to running on bare-metal hardware.

***

### **7.4 The Central Role of VMs in Cloud Computing**
Virtual machines are the enabling technology for modern cloud computing. They provide the necessary resource isolation, management flexibility, and efficiency that allow cloud providers to serve thousands of customers on shared infrastructure.
