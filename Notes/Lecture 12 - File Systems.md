# Lecture 12: File Systems

## 1. Introduction to File Systems

### 1.1 The Need for Persistent Storage
Most computer systems require a method to store data **persistently**, meaning the data remains intact and accessible even after a system reboot or complete power down. This is a core functionality for several reasons:
*   The Operating System (OS) itself must be stored persistently to be loaded during boot-up.
*   Applications and user data files (documents, programs, media) need to be saved for future use.

File systems are a fundamental and constantly used component of the OS, responsible for managing this persistent data.

***
### 1.2 Challenges of File Systems
A primary challenge in file system design is the significant **performance mismatch** between persistent storage hardware and other system components.
*   **CPU, Memory, and Bus:** Operate at nanosecond speeds.
*   **Flash Drives:** Are approximately 1000 times slower than main memory (RAM).

This speed gap means that accessing data from storage can become a major bottleneck. A key goal of file system design is to implement strategies that hide or mitigate this latency to maintain overall system performance.

***
## 2. Options for Persistent Data Management

There are three main conceptual approaches to managing persistent data on storage devices.

1.  **Use Raw Storage Blocks:** This approach would expose the raw blocks of a storage device (like a flash drive or hard disk) directly to users and developers. This is not a practical solution because it provides no organization, making it extremely difficult to track where specific data is located among millions of blocks.

2.  **Use a Database:** At the other extreme, a full-fledged database could manage the data. Databases provide rich structure, indexing, and querying capabilities. However, this level of structure comes with significant performance and storage overhead, which is unnecessary for the general-purpose data storage that file systems provide.

3.  **Use a File System:** A file system serves as a middle ground. It provides an organized way of structuring persistent data that is intuitive for both users and programmers. It abstracts the physical blocks into logical units called **files**, which can be organized into collections called **directories**. This model is inherited from the physical filing cabinet metaphor.

***
## 3. Core Concepts and Desirable Properties

### 3.1 Data and Metadata
File systems must manage two distinct kinds of information, both of which must be stored persistently:

*   **Data:** The actual information a user or application stores in a file. This can be anything from the text of a document to the executable code of a program.
*   **Metadata:** Information *about* the data. It describes the file's properties and organization. Metadata is also referred to as **attributes**.
    *   **Examples:** File name, size in bytes, creation/modification timestamps, access permissions, and pointers to the physical location of the file's data blocks on the storage device.

***
### 3.2 File Systems and Hardware
File systems are designed to work on hardware that provides persistent memory, such as flash drives, hard disk drives (HDDs), CDs/DVDs, and tapes.

*   **Hardware Abstraction:** A key goal is for the user-visible file system to be **hardware-agnostic**. An application should be able to use the same system calls (e.g., `open`, `read`, `write`) to access a file regardless of whether it's stored on a flash drive or an HDD.
*   **Performance Matching:** While the top-level interface is abstract, the low-level implementation of the file system must be tailored to the specific characteristics of the hardware to achieve optimal performance. For example, the design considerations for a flash drive are different from those for a rotating HDD.

***
#### 3.2.1 Flash Drive Characteristics
Flash drives are solid-state persistent storage devices with no moving parts. Their specific properties influence file system design:
*   **Performance:** They offer fast reads (e.g., up to 100 MB/sec) and writes (e.g., up to 40 MB/sec).
*   **Write/Erase Limitation:** A given block can only be written to once. To rewrite a block, a larger unit called a **sector** containing that block must first be erased. This erase operation is significantly slower than reading or writing and is a major performance consideration.

***
### 3.3 Desirable File System Properties
An ideal file system would exhibit the following properties. In practice, designs often involve trade-offs between them.

| Property | Description |
| --- | --- |
| **Persistence** | Data must survive power cycles and reboots. This is the fundamental requirement. |
| **Easy Use Model** | Provides a simple and intuitive model for accessing individual files and organizing them into collections (directories). |
| **Flexibility** | Avoids arbitrary limits on the number of files, file size, content type, etc. The file system should just manage a sequence of bytes. |
| **Portability** | The same file system interface should work across different types of hardware devices, or even if we don't use persistent memory at all (e.g., RAM file systems). |
| **Performance** | Operations should be as fast as possible, minimizing the performance penalty of using slower persistent storage. |
| **Reliability** | Files should not be lost or corrupted due to hardware or software errors. Data must be consistent, especially in the face of system crashes. |
| **Suitable Security** | The file system must enforce access controls, allowing data owners to specify who can access their files and in what manner (read, write, execute). |

***
## **4. File System Architecture: Design and Rationale**

File systems in modern operating systems are constructed as a series of layered abstractions to provide hardware independence, enable software portability, manage complexity, and, most importantly, mitigate the severe performance bottleneck caused by slow persistent storage devices.

***
#### **4.1 The File System API (System Call Interface)**

**What It Is:** The API is the top-level, unified interface that applications use to interact with the file system. It is composed of a standard set of system calls, which are organized into three categories:

1.  **File Container Operations:** These calls treat a file as a single object, manipulating its metadata without regard to its content.
    *   Examples: `create`, `delete`, `rename`, `create/destroy links`, `get/set attributes` (e.g., permissions, owner).
2.  **Directory Operations:** These calls manage the hierarchical organization of the file system. A directory is a special type of file that contains mappings from file names to their corresponding file control structures.
    *   Examples: `find a file by name`, `create a new file/directory`, `list directory contents`.
3.  **File I/O Operations:** These calls work with the data inside a file.
    *   `open`: Prepares a file for access and sets up necessary in-memory structures.
    *   `read`/`write`: Transfer data between a file and the application's memory space.
    *   `seek`: Modifies the current read/write position (the "cursor") within an open file.
    *   `map`: Maps a file directly into a process's address space, allowing it to be accessed like an array in memory.

**Why It Exists:** The fundamental purpose of a single, unified API is to provide **abstraction** and **portability**.
*   **Problem:** Without it, applications would need to be written with specific knowledge of the underlying hardware and file system format (e.g., using different calls for an NTFS partition versus a FAT32 USB drive). This would make software development brittle and complex.
*   **Solution:** By offering one consistent interface, the OS guarantees that any application using the API can function correctly on any supported storage device. This is essential for **program portability**, allowing software to be distributed and run widely without modification. It also promotes **developer simplicity** by hiding immense low-level complexity.

***
#### **4.1.1 Read Operation Process**
When an application requests to read data from a file:

*   **Application Request:** The application issues a `read` system call, specifying the open file, a buffer in its own memory (**user space**), and the number of bytes to read.
*   **Logical Block Calculation:** The OS kernel calculates which logical block(s) of the file contain the requested bytes.
*   **Buffer Cache Check:** The kernel first checks the system's buffer cache to see if these blocks are already present in RAM.
    *   **Cache Hit:** If a block is found in the cache, the slow step of accessing the physical storage device is skipped.
    *   **Cache Miss:** If a block is not in the cache, the OS issues a request to the device driver to fetch the block from the physical storage device and load it into a **system buffer** within the cache. The application must wait for this I/O operation to complete.
*   **Data Transfer:** The kernel copies the requested data from the system buffer (now in the cache) to the application's user-space buffer.
*   **Return Control:** The `read` system call returns, and the application can now use the data in its buffer.

***
#### **4.1.2 Write Operation Process**

When an application requests to write data to a file:

*   **Application Request:** The application issues a `write` system call, providing the open file, a user-space buffer containing the data to be written, and the number of bytes.
*   **Logical Block Calculation:** The OS kernel calculates which logical block(s) of the file the data should be written to.
*   **Buffer Cache Interaction:** The kernel ensures the target block is in a system buffer within the cache. If the write does not cover an entire block, the block may first need to be read from the device.
*   **Data Transfer:** The kernel copies the data from the application's user-space buffer into the system buffer. This buffer in the cache is now marked as **"dirty,"** indicating its contents are more recent than what is on the physical disk.
*   **Immediate Return (Write-Back Caching):** Typically, the `write` system call returns control to the application *immediately* after the copy to the cache is complete. The application perceives the write as finished, even though the data has not yet been physically saved to the storage device.
*   **Asynchronous Write-Back:** At a later, more optimal time, the OS requests the device driver to **write back** the contents of the dirty buffer to the physical storage device. This operation happens asynchronously in the background, allowing the application to continue its work without waiting for the slow device I/O.

***
#### **4.2 The Virtual File System (VFS) Layer**

**What It Is:** The VFS, also known as a **federation layer**, is an abstraction layer that sits directly beneath the API. Its function is to manage multiple, concurrent file system implementations. It acts as a "plug-in" manager and intelligent router. When a system call arrives from the API, the VFS determines which file system is responsible for the target file (e.g., based on which device it resides on) and dispatches the call to the correct implementation module (e.g., the NTFS driver, the EXT4 driver).

**Why It Exists:** The VFS is the key to supporting a heterogeneous storage environment.
*   **Problem:** A single machine often uses several different file systems simultaneously. The OS needs a way to allow the single API to work seamlessly across all of them.
*   **Solution:** The VFS architecture allows different file system types to coexist transparently. Its "plug-in" nature provides:
    *   **Modularity:** New file systems can be added to the OS by creating a module that adheres to the VFS interface, without changing the OS core or applications.
    *   **Implementation Transparency:** The VFS hides the vast internal differences between file systems (e.g., FAT's linked-list vs. Unix's inodes) from the layers above it.
    *   **Flexibility:** It can extend the file metaphor to non-file entities, routing calls to device drivers or network socket managers.

***
#### **4.3 The File Systems Layer**

**What It Is:** This layer consists of the specific, concrete implementations of different file systems (e.g., EXT4, NTFS, FAT32, APFS). These are the modules managed by the VFS. Each implementation is responsible for the core logic of its particular file system format. Key functions performed at this layer include:
*   **Logical to Physical Mapping:** Translating a logical file offset into a specific block address on a device. This is its most critical function: mapping `<file, offset>` to `<device, block>`.
*   **On-Device Structure Management:** Interpreting and manipulating the file system's on-device data structures, such as inodes, File Allocation Tables, or B-trees.
*   **Free Space Management:** Tracking which storage blocks are in use and which are available for allocation.
*   **File Lifecycle Management:** Handling the logic for creating, destroying, and extending files.
*   **Metadata Management:** Reading and writing file attributes like size, timestamps, and permissions.

**Why It Exists:** This layer exists to provide **specialization**.
*   **Problem:** A single, one-size-fits-all file system design is suboptimal because different storage technologies and use cases have conflicting requirements.
*   **Solution:** Allow for multiple, specialized file system implementations to coexist, each tailored for a specific purpose.
*   **Rationale:**
    *   **Performance Optimization:** Implementations can be optimized for the specific characteristics of the underlying hardware (e.g., flash memory's write behavior vs. a rotating HDD's seek times).
    *   **Tailored Services:** Different file systems can offer different features and guarantees. Some prioritize high reliability and consistency (e.g., journaling file systems), while others prioritize raw speed (e.g., a temporary RAM-based file system).
    *   **Special Use Cases:** This enables specialized formats like read-only file systems for CDs/DVDs (ISO 9660) or network file systems (NFS).
    *   **Interoperability:** It allows an OS to read and write to storage devices formatted by other operating systems (e.g., a Linux system accessing a Windows NTFS partition).

***
#### **4.4 The Device Independent Block I/O Layer**

**What It Is:** This layer serves as the universal intermediary between all file system implementations and the underlying hardware device drivers. Its core purpose is to present a clean, generic, and high-performance block storage model to the rest of the OS. It accomplishes this through several key functions:

*   **Unified Buffer Cache:** Its most critical component is a single, system-wide cache in RAM that holds recently used blocks. This cache stores not only user data blocks but also frequently accessed **metadata blocks**, such as inodes, indirect blocks, and directory blocks.
*   **Generic Block Interface:** It abstracts all storage hardware into a simple, linear array of logical blocks. File systems interact with this logical view (e.g., "read logical block #5000") without needing to know the specific hardware commands or physical geometry of the device.
*   **I/O Scheduling:** It maintains a queue of pending read and write requests and can reorder them to optimize device throughput. For rotating disks (HDDs), this often involves an "elevator algorithm" to minimize physical head movement (seek time).
*   **Asynchronous I/O Management:** It manages the process of reading blocks into the cache ahead of time (**read-ahead/prefetching**) and writing modified ("dirty") blocks from the cache back to the device asynchronously.
*   **Data Re-blocking:** It resolves any mismatch between the OS's logical block size (e.g., 4KB) and the device's physical sector or page size (e.g., 512 bytes or 8KB).

**Why It Exists:** This layer is fundamentally about **performance optimization** and **hardware abstraction**.

*   **Problem:** Persistent storage devices are orders of magnitude slower than RAM. Furthermore, their performance characteristics are complex and varied. Directly exposing this slowness and complexity to every file system implementation would make them inefficient, difficult to write, and non-portable.

*   **Solution & Rationale:**

    1.  **To Maximize Performance via Caching:**
        *   **Read Caching & Prefetching:** The layer mitigates read latency by caching blocks. For sequential file access, its **read-ahead** logic proactively fetches subsequent blocks into the cache before they are requested. When the application does request them, the data is already in RAM, resulting in near-instantaneous access.
        *   **Write-Back Caching:** This technique dramatically improves application responsiveness. When an application writes data, the layer simply copies it to a cache buffer, marks the buffer as "dirty," and immediately returns control to the application. The slow physical write to the device occurs later, in the background. This also allows the OS to **coalesce** multiple small writes to the same block into a single physical write and to **schedule** disk activity during idle periods.
        *   **Unified Cache Efficiency:** A single, system-wide cache is more effective than multiple separate caches because it creates a larger, global pool of memory. It allows for efficient **inter-process sharing** (if two processes access the same file, the second gets a cache hit from the first's activity) and ensures that high-traffic **metadata blocks** remain cached, benefiting the entire system.

    2.  **To Provide True Hardware Abstraction:**
        *   **Simplifying File Systems:** By providing a generic block interface, this layer isolates file system developers from hardware-specific complexity. The EXT4 implementation, for instance, does not need to know how to issue NVMe or SCSI commands; it simply requests a logical block number, and this layer, in concert with the device driver, handles the translation.
        *   **Centralizing Optimization Logic:** Complex logic like I/O scheduling doesn't need to be reimplemented in every file system. It is handled once in this common layer, ensuring all file systems benefit from device optimization.

***
## **5. File System Control Structures**

At the heart of any file system is the set of data structures it uses to manage files. These structures must track a file's metadata and, most critically, the location of its data blocks on a storage device. The design of these control structures is a core element that profoundly impacts a file system's capabilities and performance.

***
### **5.1 The Role and Challenges of Control Structures**

A file is a named collection of information, and the file system has two primary roles regarding its control structures:
1.  **Store and Retrieve Data:** The structures must provide a map to locate the data blocks associated with a file.
2.  **Manage Media Space:** They must track which blocks on the device are allocated to which files and which are free.

To fulfill these roles, the control structures must efficiently support a set of **typical operations**:
*   Find the first block of a file.
*   Given the current block, find the next sequential block.
*   Directly find an arbitrary block (e.g., "Where is block 35 of this file?").
*   Allocate a new block to the end of a file to allow it to grow.
*   Free all blocks associated with a file when it is deleted.

Designing structures to handle these operations presents significant **space management challenges**:
*   **Scale:** Devices can have millions of blocks and thousands of files.
*   **Dynamic Nature:** Files are constantly created, destroyed, and extended after their initial creation.
*   **Performance:** The physical placement of data blocks can have major performance effects, and poor management leads to poor overall system performance.

***
### **5.2 The Basic File Control Structure Problem**

Any viable design for a file control structure must address these fundamental requirements:
*   **Findability:** Since a file typically consists of multiple data blocks, the control structure must provide a way to find all of them.
*   **Fast Random Access:** It should be possible to find any block quickly, without needing to read the entire file to find a block near the end.
*   **Mutability:** The contents of blocks can be changed. The structure must support updating data in place or remapping a logical block to a new physical location.
*   **Dynamic Sizing:** The structure must accommodate files growing (adding new data) or shrinking (deleting old data).

***
### **5.3 On-Device vs. In-Memory Structures**

To solve this problem, file systems use a two-part approach involving both persistent and volatile structures.

*   **On-Device Structure (`dinode`):** A persistent data structure stored on the physical storage device for every file. Its primary function is to hold the file's permanent attributes (size, permissions, timestamps) and the crucial pointers to the file's physical **device blocks**. This is the authoritative, on-disk record.
*   **In-Memory Structure (`inode`):** When a file is opened, the OS creates a temporary representation of its control structure in RAM. This is not an exact copy.
    *   It points to **RAM pages** within the buffer cache where the file's blocks are currently loaded, or indicates that a block is not in memory.
    *   It tracks transient state, such as which cached blocks are **"dirty"** (have been modified in RAM but not yet written back to the device).
    *   It contains a cursor or file offset pointer to track the current read/write position.

***
### **5.4 The "Per-Process or Not?" Dilemma**

A critical design question is how to manage in-memory structures when multiple processes open the same file.
*   **The Cursor Problem:** If all processes share a single in-memory structure, they would also share a single file cursor (read/write pointer). If one process reads 100 bytes, the cursor moves for all processes, which is usually undesirable for independent processes. This suggests a per-process structure is needed.
*   **The Cooperation Problem:** Sometimes, cooperating processes *do* want to share a file cursor to work together on a file. This suggests a shared structure is needed.
*   **The Resource Management Problem:** How does the OS know when to deallocate the in-memory structures? It can only do so when the *last* process has closed the file. This requires some form of shared tracking.

This dilemma—the need for both per-process state and shared state—leads directly to a multi-level solution.

***
### **5.5 The Unix Multi-Level Approach**

The Unix/Linux approach solves this dilemma by using a hierarchy of three in-memory structures for each open file.

1.  **User File Descriptor (per-process):** This is an entry in a per-process table. It is simply a small integer that the application uses to refer to an open file. It acts as a pointer to the next layer.
2.  **Open File Instance Descriptor (shared by cooperating processes):** This structure contains the state for a specific `open()` instance, including the file's current read/write offset (the cursor) and the mode it was opened with (e.g., read-only).
    *   Normally, each process gets its own instance descriptor.
    *   Cooperating processes can be explicitly set up to share a single instance descriptor, which allows them to share a file cursor.
3.  **In-Memory Inode (`struct inode`) (shared by all):** There is only **one** of these per open file in the entire system. It is shared by all open file instance descriptors that refer to that file. It contains the file's attributes (size, owner) and pointers to its data blocks in the buffer cache. It also maintains a reference count to know when the last process has closed the file, at which point it can be deallocated.
4.  **On-Disk Inode (`struct dinode`) (The Persistent Record):** This is the foundational layer. It is the permanent, physical data structure stored on the disk for every file.
    *   When a file is opened, the kernel reads the `dinode` from the disk to create and populate the in-memory `inode`.
    *   When a file's metadata is changed (e.g., permissions are modified or the file grows), the changes are made to the in-memory `inode` and are eventually written back to the `dinode` on disk to ensure the changes persist across reboots. This is the authoritative record of the file.

This hierarchy elegantly provides both per-process privacy (separate cursors by default) and mechanisms for sharing, all while centralizing resource management.

***
## **6. On-Device File System Organization**

The theoretical layers of a file system must ultimately be realized as a concrete data structure on a physical storage device. This section explores how a device's linear sequence of blocks can be organized to create a functional file system, focusing on two classic, influential designs.

***
### **6.1 The Basics of On-Device Structure**

Before examining specific examples, several universal principles apply to most on-device file system layouts:

1.  **Block-Oriented Foundation:** File systems are built on block-oriented devices, which are divided into fixed-size blocks (e.g., 512, 1024, 4096 bytes).
2.  **The Boot Block (Block 0):** By convention, the first block of a storage device is reserved as the **boot block**. It contains a small program that the system firmware loads to initiate the OS boot process. The file system itself typically ignores Block 0 and begins its work at Block 1.
3.  **Data and Metadata Division:** The device's blocks are partitioned to store two kinds of information:
    *   **User Data:** The contents of files. The majority of blocks are used for this purpose.
    *   **Organizing Metadata:** Data that describes the file system. This includes:
        *   **A file system descriptor** (e.g., a "super block") that specifies the layout, size, and state of the entire volume.
        *   **File control blocks** (e.g., "inodes" or directory entries) that describe individual files.
        *   **A free space map** which tracks all the blocks that are not yet allocated to any file.

***
### **6.2 Managing Allocated Space: The Core Problem**

A core activity for any file system is managing the allocation of its blocks. Early design considerations mirror those from virtual memory management:

*   **Fixed-Sized Allocation:** If every file were allocated the same fixed number of blocks, it would suffer from severe **internal fragmentation**, wasting space for small files and making large files impossible.
*   **Variable-Sized Allocation:** If every file were allocated exactly the number of bytes it needed, the device would suffer from **external fragmentation**, creating unusable "holes" between files that would require slow compaction routines.

The universal solution is to allocate space in fixed-size chunks—the device's **blocks** (or multiples of blocks called **clusters**). This leads to a small amount of internal fragmentation in the last block of a file but avoids external fragmentation. This choice, however, creates a new problem: how does the file system keep track of all the potentially non-contiguous blocks that belong to a single file? The following designs are two different answers to this question.

***
#### **6.2.1 Linked Extents**

One of the simplest answers to the question of how to track a file's blocks is to connect them like a linked list. In this **linked extent** (or linked allocation) model, the file's control block needs to contain only one piece of information: a pointer to the very first "chunk" or "extent" (a block or group of blocks) of the file.

From there, there are two primary ways to find the subsequent chunks:

1.  **Pointers Within Data Chunks:** Each data chunk reserves a small amount of space at its end to store a pointer to the next chunk in the sequence.
    *   **Advantage:** Very simple to implement.
    *   **Disadvantage:** This method is extremely inefficient for random access. To find the Nth chunk of a file, you must perform N-1 slow disk I/O operations to read through the preceding chunks just to follow the chain of pointers.

2.  **Pointers in an Auxiliary Table:** All the linkage pointers for the entire volume are stored together in a separate, dedicated table on the device. This "chunk linkage" table is loaded into RAM when the system boots.
    *   **Advantage:** Random access is much faster. To find the Nth chunk, the system can traverse the chain of pointers entirely within fast memory, requiring only a single disk I/O to fetch the final target data chunk.
    *   **Disadvantage:** The entire linkage table must fit in memory, which can constrain the maximum size of the storage volume.

The DOS FAT file system is a classic and widely used implementation of this second, table-based linked extent approach.

### **6.3 Method 1: Linked Extents (The DOS FAT File System)**

The File Allocation Table (FAT) file system, pioneered by Microsoft, uses a structure analogous to a linked list to connect a file's data blocks.

***
#### **6.3.1 DOS FAT On-Device Layout**

A volume formatted with FAT has a specific, sequential layout:
1.  **Block 0: Boot Block.**
2.  **Block 1: BIOS Parameter Block (BPB).** This is a critical metadata block that describes the volume's geometry, including the size of a cluster and the size and location of the FAT itself.
3.  **File Allocation Table (FAT):** Following the BPB is the FAT itself. This is a large table that acts as the central "chunk linkage" map for the entire volume.
4.  **Data Area:** The remainder of the device holds the data **clusters**. By convention, the first data cluster is the **root directory**.

***
#### **6.3.2 How FAT Works**

*   **Directory Entry as File Descriptor:** A file's control block is its entry in a directory. This entry contains the file's name, size, and a pointer to its **first cluster**.
*   **The FAT as a Linked-List Map:** The FAT is an array where each index corresponds to a cluster number on the device. The value stored at a given entry indicates the status of that cluster or the next cluster in a file's chain.

To find the blocks of a file, the system follows a chain of pointers through the in-memory FAT. The special values in the table are key to its operation:
*   `FAT[i] = j` (where j > 0): The cluster that follows cluster `i` is cluster `j`.
*   `FAT[i] = 0`: This indicates that cluster `i` is **free** and available for allocation.
*   `FAT[i] = -1` (or another special reserved value): This marks cluster `i` as the **end of file (EOF)**; it is the last cluster in its chain.

**Example Walkthrough:**

Imagine a file system with the following state in its File Allocation Table and a directory entry for `myfile.txt`:

**Directory Entry:**
*   **Name:** `myfile.txt`
*   **Length:** 1500 bytes (assuming 512-byte clusters)
*   **1st Cluster:** `3`

**File Allocation Table (FAT) - Partial View:**

| Index | Value | Meaning |
|---|---|---|
| ... | ... | ... |
| 2 | `0` | Cluster #2 is free. |
| **3** | `4` | The cluster after #3 is #4. |
| **4** | `5` | The cluster after #4 is #5. |
| **5** | `-1` | Cluster #5 is the End of the File. |
| 6 | `0` | Cluster #6 is free. |
| ... | ... | ... |

**File Reading Process:**
1.  The system looks at the directory entry for `myfile.txt` and finds that its first cluster is **3**. It reads the first 512 bytes of the file from cluster 3.
2.  To find the next part of the file, the OS looks at `FAT[3]`. The value is **4**. This means the next cluster is #4. The system reads the next 512 bytes from cluster 4.
3.  The OS then looks at `FAT[4]`. The value is **5**. This means the next cluster is #5. The system reads the remaining 476 bytes from cluster 5.
4.  After reading from cluster 5, the OS checks `FAT[5]` and finds the value **-1**, the EOF marker. It now knows there are no more clusters associated with this file.

**File Allocation Process:**
If the system needed to allocate a new cluster (e.g., to create a new file), it would scan the FAT for a `0` entry. In this example, it would find that cluster #2 and cluster #6 are available.

***
#### **6.3.3 FAT Characteristics and Limitations**

*   **Performance:** Because the entire FAT is kept in memory, following the chain of pointers to find the next cluster is very fast. However, finding an arbitrary block deep inside a very large file requires a long chain of lookups in the in-memory table. No external fragmentation. Internal fragmentation is limited to the size of one cluster, minus one byte, per file. 
*   **File Size Limitation:** Random access to a block deep inside a large file requires traversing the entire chain of pointers in the FAT. The size of the file system is limited by the number of entries the FAT can hold and the number of bits used for each entry. The original FAT12 with 4096 entries and 512-byte clusters limited the maximum file size to 4 MB. 

***
### **6.4 Method 2: File Index Blocks**

An alternative and more scalable approach to file allocation is to use a central **file index block** for each file. This structure, often called an **inode** (index node), consolidates all the information needed to find a file's data.

***
#### **6.4.1 The Core Concept and Its Limitation**

The basic idea of a file index block is simple: create a single control block for each file that contains an array of pointers to *all* of that file's data blocks.

*   **Advantage:** This provides extremely fast **random access**. To find the 35th block of a file, the system can simply read the main index block and look up the 35th pointer in the array, requiring only one I/O operation to find the block's location.
*   **Fundamental Limitation:** A simple index block has a fixed size and can therefore only hold a fixed number of pointers. This directly imposes a hard limit on the maximum number of blocks a file can have, and thus, a hard limit on the file's size.

***
#### **6.4.2 The Solution: Hierarchically Structured Index Blocks**

To solve the file size limitation of a simple index block, modern file systems use a **hierarchical structure**. This approach allows the file system to adapt to a vast range of file sizes, from very small to enormous.

The mechanism works by designating different roles for the pointers within the main index block (the inode):
*   Some pointers point **directly** to data blocks, providing fast access for small files.
*   Other pointers are reserved to point to **indirect blocks**. An indirect block is not a data block; it is a full block used exclusively to store an array of more pointers.
*   This indirection can be layered, creating **double** and **triple indirect blocks** for immense scalability.

This design offers a powerful trade-off: small files are highly efficient to access, while large files are still possible, incurring a small, manageable I/O overhead to fetch the necessary indirect pointer blocks.

***
#### **6.4.3 Concrete Example: The Unix System V File System**

The Unix file system is the classic implementation of a hierarchically structured index block design.

**On-Device Layout:**
1.  **Block 0: Boot Block.**
2.  **Block 1: Super Block.** Describes the file system's overall parameters, such as block size and the total number of inodes.
3.  **I-node List:** A contiguous region of the disk reserved exclusively for storing all the **on-disk inodes (`dinodes`)** for the volume.
    *   **Clarification on `dinode` vs. `inode`:** The term `dinode` refers to the **persistent, on-disk structure** that holds a file's permanent metadata. When a file is opened, the kernel reads its `dinode` from this list into memory and creates a more feature-rich **in-memory inode (`inode`)** to manage the active file. The I-node List on the physical device therefore contains the `dinodes`.
    *   Within this list, **I-node #1** is special by convention: its `dinode` describes the **root directory**, providing the starting point for navigating the entire file system hierarchy.
4.  **Data Blocks:** The remainder of the device space, available for file and directory data. The lecture slide explicitly states, "Data blocks begin immediately after the end of the I-nodes."

***
#### **6.4.4 The Hierarchical Inode Structure:**

**The Idea: A Tiered, Asymmetrical Structure**
Based on the observation that the vast majority of files are small (scripts, configs, source code), while very few are enormous (videos, databases), the designers implemented an asymmetrical pointer structure to optimize for the common case.

1.  **Tier 1: Direct Pointers (The "Fast Path")**
    *   **Purpose:** The direct pointers provide an extremely low-overhead "fast path" for accessing the beginning of any file. For any file up to the size covered by these direct blocks (e.g., 40KB), the location of any block can be found with a single I/O operation: reading the inode from disk.

2.  **Tier 2: Indirect Pointers (The "Scalable Path")**
    *   **Purpose:** This provides the mechanism for handling the exceptional case of larger files. The system only pays the performance penalty of reading extra pointer blocks when it is absolutely necessary—that is, when a file grows beyond the capacity of the direct pointers.

*   **Direct Pointers (e.g., 10):** The first several pointers in the inode point directly to the first few data blocks of the file.
*   **Single Indirect Pointer:** The next pointer points to an **indirect block** which contains pointers to 1024 blocks.
*   **Double Indirect Pointer:** The next pointer points to a **double indirect block** which contains pointers to 1024 indirect blocks.
*   **Triple Indirect Pointer:** The final pointer points to a **triple indirect block** which contains pointers to 1024 double indirect blocks.

**Consequences of the Design:**
*   **High Performance for Common Case:** For small files, access cost is minimal (one I/O for the inode, one for the data), making operations on the most numerous file types exceptionally fast.
*   **Graceful Scalability:** The design handles massive files (up to Terabytes) without sacrificing small-file efficiency. The transition is graceful; the system only incurs the cost of indirection as the file actually needs it.
*   **Increased Kernel Complexity:** The trade-off is more complex kernel code to translate a logical file offset into a physical block address, as it must check each tier of the hierarchy.
*   **Amortized Cost for Large Files:** While fetching the first block in an indirect range requires extra I/O, that single fetched indirect block (once cached) contains pointers to thousands of subsequent data blocks, making the *average* cost per block remain very low.

***
#### **6.4.5 Characteristics and Performance Summary**

*   **Scalability:** Assuming 4KB blocks and 4-byte pointers, the 13-pointer structure can address a file of up to **4 Terabytes**.
    *   **10 Direct Blocks:** 10 * 4KB = **40KB**
    *   **Single Indirect:** 1K pointers * 4KB = **4MB**
    *   **Double Indirect:** 1K * 4MB = **4GB**
    *   **Triple Indirect:** 1K * 4GB = **4TB**
*   **Performance:**
    *   Access to the first 40KB is extremely efficient.
    *   Any data block in a 4TB file can be found with at most **3 extra reads** (for the triple, double, and single indirect blocks).
    *   The cost of these extra reads is amortized because the indirect blocks are kept in the **buffer cache**.

***
## 7. Conclusion
File systems are the OS's preferred method for managing persistent data. They must be designed to balance the competing goals of performance, reliability, and security. Modern operating systems use a layered VFS architecture to support multiple, diverse file system designs. Key design aspects, such as the choice between a linked-list structure (FAT) and an indexed structure (inodes), have a profound and lasting impact on the capabilities and performance of the file system.
