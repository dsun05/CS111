# Lecture 11: Devices, Device Drivers, and I/O

## 1. Introduction to Peripheral Devices

A computer consists of more than just a CPU, memory, and a bus. It includes numerous other components known as **peripheral devices**, which handle Input/Output (I/O) operations.

- **Examples of Peripheral Devices**:
    - Monitors
    - Keyboards & Mice
    - Hard disk drives & flash drives
    - Speakers & Microphones
    - Network adapters (Wi-Fi, Ethernet)
    - Printers & Scanners

Each peripheral device requires associated code to perform its specific operations and to integrate it with the rest of the system. In modern commodity operating systems, the volume of code dedicated to handling these devices significantly exceeds the code for all other OS functions combined.

### 1.1. Role of Peripherals in a Computer System

1.  **Attachment**: Most peripherals are attached to a **bus**, which is a communication system that transfers data between components inside a computer. This allows other components, like the CPU and memory, to communicate with them.
2.  **Function**: Peripherals are specialized hardware built to perform a limited set of specific commands, not for general-purpose computation.
3.  **Operation**: The OS sends signals over the bus to a peripheral, ordering it to perform an action. This operation is typically performed **asynchronously**, meaning the device works independently and in parallel with the CPU.
4.  **Completion**: Once the device has completed its task, it sends a signal back over the bus to the CPU to indicate the result or that the operation is finished.

## 2. Device Drivers

A **device driver** is a specialized piece of software that controls a particular type of hardware device. It acts as a translator between the device and the programs or operating systems that use it.

### 2.1. The Role and Position of a Device Driver

A device driver sits between a high-level application and a low-level hardware device, with the operating system mediating the interaction.

- **High-Level Application**: A user-facing program, like a web browser, that needs to perform an I/O operation (e.g., sending a network request).
- **Low-Level Hardware**: A specific piece of hardware, like an "Intel Gigabit CT PCI-E Network Adapter," that performs the physical I/O.
- **Operating System**: When the application requests an action, the OS invokes the appropriate device driver.
- **Device Driver**: The driver takes the high-level command from the OS and translates it into a series of detailed, low-level instructions and signals that the specific hardware understands.

### 2.2. Typical Properties of Device Drivers

- **Specificity**: A driver is highly specific to the particular hardware model it was written for. Code for a mouse is different from code for a monitor. Furthermore, code for one model of a mouse may be different from the code for another model.
- **Modularity**: Drivers are designed to be independent modules that can be "plugged into" the OS as needed. A system only needs the drivers for the devices it actually hosts.
- **Defined Interaction**: They interact with the rest of the OS in limited, well-defined ways through established interfaces.
- **Correctness**: The correctness of a device driver is critical. A bug in a driver can crash the entire operating system, as drivers typically run in the privileged kernel space.
- **Authorship**: Drivers are generally written by programmers who work for the device manufacturer. These programmers have deep knowledge of the hardware but are not necessarily experts in general operating system design.

### 2.3. Why the OS Manages Devices

Device control logic is typically handled within the OS kernel rather than in user-level code for several key reasons:

1.  **System Correctness**: Some devices are critical for core OS functions. For example, the disk or flash drive holding the swap space for virtual memory must be managed by trusted OS code to prevent corruption.
2.  **Resource Sharing**: Devices like screens, printers, and network cards must be shared among multiple processes. The OS must manage this sharing to prevent conflicts and ensure orderly access.
3.  **Security**: Devices may handle sensitive data. If a process's memory is paged out to disk, that data must be protected from access by other, unauthorized processes. Placing device control within the OS enforces these security boundaries.

## 3. I/O Performance

A major challenge in I/O management is the vast performance difference between system components.

- **Speed Mismatch**: CPUs, buses, and RAM operate at extremely high speeds. In contrast, most peripheral devices are several orders of magnitude slower.
- **Performance Challenge**: The system must operate at CPU speeds, but application correctness often depends on interactions with these slow devices.
- **OS Role**: The OS system code is responsible for managing this speed mismatch to maximize overall system performance and responsiveness.

### 3.1. The Problem of Poor I/O Device Utilization

Inefficient I/O management leads to periods where the device is idle, even when there is work to be done. This severely degrades performance.

**Example Sequence of Poor Utilization:**
1.  A process runs and prepares for an I/O operation. The device is idle.
2.  The process issues a system call (e.g., a read). The process blocks, awaiting completion.
3.  The OS processes the call and commands the device to start.
4.  The device becomes busy and performs the requested operation.
5.  The device finishes and sends a completion interrupt. The device becomes idle again.
6.  The OS handles the interrupt and awakens the blocked process.
7.  The process runs again, processes the result, and does more computation. The device remains idle.
8.  The process issues another read, starting the cycle over.

In this scenario, the device is only busy for a fraction of the total time, leading to low throughput.

### 3.2. Improving I/O Performance

The key to better performance is to exploit **parallelism** and keep key devices as busy as possible.

#### 3.2.1. Direct Memory Access (DMA)

Modern CPUs use on-chip caches to minimize trips to main memory (RAM). This means the main memory bus is often idle from the CPU's perspective. This idle time can be used by peripherals.

- **Definition**: **Direct Memory Access (DMA)** is a feature that allows I/O devices to transfer data directly to or from main memory, without involving the CPU in the actual data movement.
- **Mechanism**: The CPU initiates the transfer by providing the device controller with the memory address, the amount of data, and the direction of transfer. The device then manages the transfer over the bus. When finished, it interrupts the CPU.
- **Advantage**: This frees the CPU to perform other tasks while the data transfer occurs in parallel. Data moves at the speed of the bus and memory, not limited by CPU instruction cycles for copying.

#### 3.2.2. Keeping Key Devices Busy

To achieve high utilization, the OS employs several strategies:
1.  **Queue Requests**: Allow multiple I/O requests to be pending for a device at a time. These are placed in a queue, similar to the ready queue for processes.
2.  **Use DMA**: Perform the actual data transfers using DMA to minimize CPU overhead and allow for parallelism.
3.  **Interrupt-Driven Workflow**: When the currently active request completes:
    - The device controller generates a completion interrupt.
    - The OS interrupt handler is called.
    - The handler posts completion to the original requester (unblocking it).
    - Crucially, the handler immediately selects the **next** transfer from the queue and initiates it on the device.

This ensures that the device starts the next operation almost immediately after finishing the previous one, minimizing idle time.

### 3.3. The Importance of Transfer Size

For I/O operations, larger data transfers are significantly more efficient than smaller ones.

- **Per-Operation Overhead**: Every transfer incurs fixed overhead costs, regardless of size. These include:
    - OS and driver instructions to set up the operation.
    - Time for the device to start a new operation.
    - CPU cycles to service the completion interrupt.
- **Efficiency**: With a larger transfer, this fixed overhead is amortized over more bytes, resulting in a lower overhead-per-byte and higher overall throughput. The graph of PCIe throughput versus transfer size clearly shows that throughput increases dramatically with transfer size before leveling off.

### 3.4. I/O and Buffering

Because data must reside in memory before being sent to a device or after being received from one, the OS must manage memory areas called **buffers**.

- **OS Role**: The OS must ensure that buffers are available when needed.
- **Consolidating Requests**: Applications often make small I/O requests (e.g., writing a few bytes at a time). To improve efficiency, the OS can consolidate these small requests.
    - **Write-back Caching**: Accumulate small writes in a memory buffer. The OS only flushes the buffer to the physical device when it is full or after a certain time has elapsed.
    - **Read-ahead**: If an application is reading a file sequentially, the OS can proactively read subsequent blocks of the file into buffers before they are explicitly requested. This hides the latency of the disk read.

### 3.5. Scatter/Gather I/O

A complication arises from the interaction between DMA and virtual memory. DMA controllers require data to be in a **physically contiguous** block of memory. However, an application's buffer in virtual memory may be spread across multiple, non-contiguous physical page frames.

- **Gather**: Writing data from multiple, non-contiguous page frames to a device in a single logical operation.
- **Scatter**: Reading data from a device into multiple, non-contiguous page frames.

There are three basic approaches to handle this:
1.  **Copying**: The OS copies the scattered user data into a special, physically contiguous kernel buffer before initiating the DMA transfer. This is simple but introduces the overhead of a memory-to-memory copy.
2.  **Chain-Scheduled Requests**: The OS breaks the single logical request into multiple smaller DMA requests, one for each contiguous physical chunk.
3.  **I/O MMU (IOMMU)**: Advanced hardware, an IOMMU, can translate device-visible virtual addresses into physical addresses, much like a CPU's MMU does for processes. This allows the device to see a contiguous buffer, while the OS can use physically scattered pages, handling scatter/gather automatically in hardware.

### 3.6. Memory-Mapped I/O

For devices that perform many small, sparse transfers (e.g., a video game display adapter), the overhead of setting up a DMA transfer for each small update is too high. An alternative is **Memory-Mapped I/O (MMIO)**.

- **Mechanism**: With MMIO, the device's control registers and internal memory are mapped into the CPU's regular physical address space.
- **Operation**: To interact with the device, the CPU simply performs ordinary load and store instructions to these specific memory addresses. There are no special I/O instructions or DMA setup.
- **Example**: To change a pixel on a memory-mapped video card, an application just writes a color value to the memory address that corresponds to that pixel's location in the card's video RAM.

| Feature | Direct Memory Access (DMA) | Memory-Mapped I/O (MMIO) |
| :--- | :--- | :--- |
| **Best For** | Occasional, large, contiguous transfers (e.g., disk I/O) | Frequent, small, sparse transfers (e.g., graphics) |
| **CPU Involvement** | Minimal (setup and interrupt only) | High (every byte is transferred by a CPU instruction) |
| **Overhead** | High per-operation setup overhead | No per-operation setup overhead |
| **Efficiency** | Very efficient for large transfers | Very inefficient for large transfers |
| **Sharing** | Relatively easy to manage and share | More difficult to share between processes |

## 4. Device Driver Abstractions

While every device model is unique, devices within the same class (e.g., all network cards, all disk drives) share many commonalities. The OS leverages these commonalities by defining **simplifying abstractions**.

### 4.1. The Role of Abstractions

Abstractions regularize and simplify the chaotic world of devices.
- **Encapsulate Knowledge**: They hide the device-specific details of usage, optimization, and fault handling from higher-level software.
- **Standardize Behavior**: They define a standard set of methods and behaviors for a class of devices, forcing diverse hardware to fit a common mold.
- **Protect Applications**: They shield applications from device eccentricities and hardware-specific error conditions.

### 4.2. Key Interfaces

To manage the relationship between the OS and thousands of potential device drivers, modern operating systems define two critical interfaces.

#### 4.2.1. Device Driver Interface (DDI)

- **Definition**: The DDI is the "top-end" interface, specifying the set of entry points and functions that a device driver must implement to be controlled *by the OS*.
- **Purpose**: It is a contract that defines what the OS expects from the driver. Third-party developers write their drivers to this specification.
- **Structure**: The DDI is often structured into a common part and class-specific parts.
    - **Common DDI**: Functions all drivers must implement (e.g., `initialize`, `cleanup`, `open`, `release`).
    - **Sub-DDIs**: Functions specific to a class (e.g., `read`, `write`, `seek` for Basic I/O; `revalidate`, `fsync` for Disks; `receive`, `transmit` for Network devices).

#### 4.2.2. Driver/Kernel Interface (DKI)

- **Definition**: The DKI is the "bottom-end" interface, specifying the set of services that the OS kernel provides *to the device drivers*.
- **Purpose**: It provides drivers with a standard way to request kernel services, analogous to an Application Binary Interface (ABI) for applications.
- **Services**: Typical DKI services include:
    - Memory allocation and management
    - Buffering services
    - DMA management
    - Synchronization primitives (mutexes, semaphores)
    - Error reporting
    - Dynamic module loading and configuration
- **Stability**: The DKI must be very stable across OS versions to ensure that old, third-party drivers continue to work on new OS releases without modification.

### 4.3. Linux Device Driver Abstractions

Linux inherits its device model from Unix, organizing devices into a class-based system with several major super-classes.

1.  **Character Devices**:
    - **Definition**: Devices that are accessed as a stream of bytes (or words). The "character" refers to a unit of data, not necessarily ASCII text.
    - **Access**: Can be sequential or random access. They typically support direct, synchronous reads and writes.
    - **Examples**: Keyboards, mice, monitors, serial ports, and most non-block/non-network devices.

2.  **Block Devices**:
    - **Definition**: Devices that are always accessed in fixed-size blocks (e.g., 4KB). You can only read or write a whole block at a time.
    - **Access**: They are random-access devices, meaning any block can be addressed directly. They support queued, asynchronous I/O.
    - **Why a Separate Class**: Block devices are fundamental to critical, high-performance system functions like file systems and virtual memory swapping. The OS provides a highly optimized and elaborate set of services (caching, scheduling) specifically for them.
    - **Examples**: Hard disk drives, solid-state drives (SSDs), CD/DVD drives.

3.  **Network Devices**:
    - **Definition**: Devices that send and receive data in discrete units called packets.
    - **Characteristics**: They are sufficiently different from other device types, as their operation is intrinsically tied to the complex rules and state management of network protocols (e.g., TCP/IP).
    - **Examples**: Ethernet cards, Wi-Fi adapters, Bluetooth devices.

### 4.4. Accessing Devices in Linux

In Linux/Unix, devices are exposed to user space through the file system, typically in the `/dev` directory.

- **Special Files**: Each device instance is represented by a special file. Standard file system calls (`open`, `read`, `write`, `close`) on this file are mapped by the OS to the corresponding entry points in the selected device driver.
- **Major and Minor Numbers**:
    - **Major Device Number**: An integer that specifies which device driver to use. All devices using the same driver share the same major number.
    - **Minor Device Number**: An integer that the driver uses to distinguish between multiple distinct hardware instances that it controls. For example, if a system has three identical disk drives, they would all have the same major number but would have minor numbers 0, 1, and 2.

**Example `ls -l` output for a block device:**
`brw-r----- 1 root operator 14, 0 Apr 11 18:03 disk0`
- `b`: Indicates a block special file.
- `14`: The major device number.
- `0`: The minor device number.
