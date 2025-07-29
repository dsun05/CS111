# Lecture 12: File Systems

## 1. Introduction to File Systems

### 1.1 The Need for Persistent Storage
Most computer systems require a method to store data **persistently**, meaning the data remains intact and accessible even after a system reboot or complete power down. This is a core functionality for several reasons:
*   The Operating System (OS) itself must be stored persistently to be loaded during boot-up.
*   Applications and user data files (documents, programs, media) need to be saved for future use.

File systems are a fundamental and constantly used component of the OS, responsible for managing this persistent data.

### 1.2 Challenges of File Systems
A primary challenge in file system design is the significant **performance mismatch** between persistent storage hardware and other system components.
*   **CPU, Memory, and Bus:** Operate at nanosecond speeds.
*   **Flash Drives:** Are approximately 1000 times slower than main memory (RAM).

This speed gap means that accessing data from storage can become a major bottleneck. A key goal of file system design is to implement strategies that hide or mitigate this latency to maintain overall system performance.

## 2. Options for Persistent Data Management

There are three main conceptual approaches to managing persistent data on storage devices.

1.  **Use Raw Storage Blocks:** This approach would expose the raw blocks of a storage device (like a flash drive or hard disk) directly to users and developers. This is not a practical solution because it provides no organization, making it extremely difficult to track where specific data is located among millions of blocks.

2.  **Use a Database:** At the other extreme, a full-fledged database could manage the data. Databases provide rich structure, indexing, and querying capabilities. However, this level of structure comes with significant performance and storage overhead, which is unnecessary for the general-purpose data storage that file systems provide.

3.  **Use a File System:** A file system serves as a middle ground. It provides an organized way of structuring persistent data that is intuitive for both users and programmers. It abstracts the physical blocks into logical units called **files**, which can be organized into collections called **directories**. This model is inherited from the physical filing cabinet metaphor.

## 3. Core Concepts and Desirable Properties

### 3.1 Data and Metadata
File systems must manage two distinct kinds of information, both of which must be stored persistently:

*   **Data:** The actual information a user or application stores in a file. This can be anything from the text of a document to the executable code of a program.
*   **Metadata:** Information *about* the data. It describes the file's properties and organization. Metadata is also referred to as **attributes**.
    *   **Examples:** File name, size in bytes, creation/modification timestamps, access permissions, and pointers to the physical location of the file's data blocks on the storage device.

### 3.2 File Systems and Hardware
File systems are designed to work on hardware that provides persistent memory, such as flash drives, hard disk drives (HDDs), CDs/DVDs, and tapes.

*   **Hardware Abstraction:** A key goal is for the user-visible file system to be **hardware-agnostic**. An application should be able to use the same system calls (e.g., `open`, `read`, `write`) to access a file regardless of whether it's stored on a flash drive or an HDD.
*   **Performance Matching:** While the top-level interface is abstract, the low-level implementation of the file system must be tailored to the specific characteristics of the hardware to achieve optimal performance. For example, the design considerations for a flash drive are different from those for a rotating HDD.

#### 3.2.1 Flash Drive Characteristics
Flash drives are solid-state persistent storage devices with no moving parts. Their specific properties influence file system design:
*   **Performance:** They offer fast reads (e.g., up to 100 MB/sec) and writes (e.g., up to 40 MB/sec).
*   **Write/Erase Limitation:** A given block can only be written to once. To rewrite a block, a larger unit called a **sector** containing that block must first be erased. This erase operation is significantly slower than reading or writing and is a major performance consideration.

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

## 4. File System Architecture in the OS

File systems are implemented as a series of layered abstractions within the OS to provide portability and modularity.



### 4.1 The File System API (System Call Interface)
This is the top-level interface exposed to applications. To ensure program portability, a single, unified API is provided for all file operations, regardless of the underlying file system or hardware. This API is composed of three main categories of system calls:

1.  **File Container Operations:** These calls treat a file as a single object, manipulating its metadata without regard to its content.
    *   Examples: `create`, `delete`, `rename`, `create/destroy links`, `get/set attributes` (e.g., permissions, owner).
2.  **Directory Operations:** These calls manage the hierarchical organization of the file system. A directory is a special type of file that contains mappings from file names to their corresponding file control structures.
    *   Examples: `find a file by name`, `create a new file/directory`, `list directory contents`.
3.  **File I/O Operations:** These calls work with the data inside a file.
    *   `open`: Prepares a file for access and sets up necessary in-memory structures.
    *   `read`/`write`: Transfer data between a file and the application's memory space.
    *   `seek`: Modifies the current read/write position (the "cursor") within an open file.
    *   `map`: Maps a file directly into a process's address space, allowing it to be accessed like an array in memory.

### 4.2 The Virtual File System (VFS) Layer
The VFS, sometimes called a **federation layer**, is a critical abstraction layer that allows the OS to support multiple different file system types simultaneously.
*   **Purpose:** It provides a "plug-in" architecture where different file system implementations (e.g., EXT4, NTFS, FAT32) can register with the OS.
*   **Function:** When an application makes a file system call, the VFS determines which underlying file system is responsible for the target file/directory and routes the call to the appropriate implementation.
*   **Benefit:** This makes the specific file system implementation transparent to applications. A program can open a file on an EXT4 partition and another on an NTFS partition using the exact same `open()` system call.

### 4.3 The Device Independent Block I/O Layer
This layer sits between the file system implementations and the hardware device drivers. Its purpose is to provide a generic, unified interface to all block storage devices.
*   **Unified Buffer Cache:** Its most critical function is to maintain a single, system-wide **buffer cache** for block data. This cache stores recently accessed blocks in RAM.
    *   **Locality of Reference:** Since file access often exhibits high spatial and temporal locality, this cache significantly improves performance by serving read requests from fast RAM instead of the slow storage device.
    *   **Efficiency:** A single shared cache has a better hit ratio and is more efficient than multiple per-process or per-device caches.
*   **Other Functions:**
    *   **Re-blocking:** Adapts the OS's block size to a device's potentially different native block size.
    *   **I/O Scheduling:** Optimizes the order of block requests sent to a device.
    *   **Error Handling:** Manages and reports errors from the device.

## 5. File System Control Structures

To manage files, the file system needs data structures to track each file's properties and data locations. These exist in two forms.

### 5.1 On-Device vs. In-Memory Structures
*   **On-Device Structure:** A persistent data structure stored on the storage device for every file. It contains crucial metadata, especially pointers to the file's data blocks. This is the authoritative record of the file. Examples include the Unix **inode** or the DOS FAT directory entry.
*   **In-Memory Structure:** When a file is opened, the OS creates a temporary representation in RAM. This structure is not an exact copy of the on-device version. It points to blocks that have been loaded into the buffer cache and tracks the state of the open file instance, such as the current read/write cursor position.

### 5.2 The Unix Approach to Multiple Processes
When multiple processes open the same file, the OS needs to manage shared and per-process state. The Unix model uses a multi-level structure:

1.  **Process Descriptor:** Each process has a table of its open file descriptors.
2.  **Open File Instance Descriptor:** Contains information for a specific `open()` call, including the read/write mode and the current file offset (cursor). Cooperating processes can be set up to share one of these to share a cursor.
3.  **In-Memory Inode:** There is only one in-memory inode per file, shared by all processes that have that file open. It contains information like file size and pointers to cached blocks.
4.  **On-Disk Inode:** The persistent structure on the disk.



## 6. On-Device File System Organization

This section details two classic methods for organizing a device's blocks into a file system.

### 6.1 Method 1: Linked Extents (The DOS FAT File System)
This approach uses a linked-list concept to chain together the blocks (called **clusters** in DOS) belonging to a file.

#### 6.1.1 DOS FAT Structure
A DOS-formatted volume is organized as follows:
*   **Block 0:** The Boot Block.
*   **Block 1:** The BIOS Parameter Block (BPB), defining file system parameters like cluster size.
*   **File Allocation Table (FAT):** A large table that acts as the "chunk linkage" mechanism. This entire table is loaded into memory at boot time.
*   **Data Area:** The remaining space, holding clusters for directories and files. The first cluster always contains the root directory.

#### 6.1.2 How FAT Works
*   A **directory entry** acts as the file descriptor. It contains the file's name, size, and a pointer to its **first** cluster.
*   The **File Allocation Table (FAT)** has one entry for every cluster on the device. The value at a given entry indicates the next cluster in a file's chain.
    *   `FAT[i] = j`: The cluster that follows cluster `i` is cluster `j`.
    *   `FAT[i] = 0`: Cluster `i` is free.
    *   `FAT[i] = -1` (or special marker): Cluster `i` is the last cluster of a file.

**Example:** To read a file whose first cluster is `3`:
1.  Read data from cluster `3`.
2.  To find the next part of the file, look at `FAT[3]`. If `FAT[3] = 4`, the next cluster is `4`.
3.  Read data from cluster `4`.
4.  To find the next part, look at `FAT[4]`. If `FAT[4] = 5`, the next cluster is `5`.
5.  Continue this process until an end-of-file marker is found in the FAT.



#### 6.1.3 FAT Characteristics
*   **Pros:** Simple design. No external fragmentation. Finding the next cluster is fast because the entire FAT is in memory.
*   **Cons:** Random access to a block deep inside a large file requires traversing the entire chain of pointers in the FAT. The size of the file system is limited by the number of entries the FAT can hold and the number of bits used for each entry. The original FAT12 with 4096 entries and 512-byte clusters limited the maximum file size to 2 MB.

### 6.2 Method 2: File Index Blocks (The Unix System V File System)
This approach uses a dedicated file control block, called an **inode** (index node), to hold an array of pointers to the file's data blocks.

#### 6.2.1 Unix System V Structure
A Unix volume is organized as follows:
*   **Block 0:** The Boot Block.
*   **Block 1:** The Super Block, describing the file system's geometry (block size, number of inodes, etc.).
*   **I-node List:** A contiguous region containing all the on-disk inodes for the file system.
*   **Data Blocks:** The remaining space for file and directory data.

#### 6.2.2 The Hierarchical Inode Structure
To overcome the file size limit of a fixed number of pointers, the Unix inode uses a hierarchical, multi-level indexing scheme. The 13 pointers in a System V inode are used as follows:
*   **Direct Pointers (10):** The first 10 pointers point directly to the first 10 data blocks of the file. This provides extremely fast access for small files, which are the most common case.
*   **Single Indirect Pointer (1):** The 11th pointer points to an **indirect block**. This block does not contain file data; instead, it is filled with direct pointers to more data blocks.
*   **Double Indirect Pointer (1):** The 12th pointer points to a **double indirect block**, which is a block filled with pointers to single indirect blocks.
*   **Triple Indirect Pointer (1):** The 13th pointer points to a **triple indirect block**, which is a block filled with pointers to double indirect blocks.



#### 6.2.3 Unix Inode Characteristics
*   **Pros:** Extremely scalable. It handles very small files efficiently while also being able to address enormous files. With 4KB blocks and 4-byte pointers, this structure can address a file up to 4 Terabytes in size. It provides fast random access, as any block can be located with at most 3 extra I/O operations (to fetch the indirect blocks), which can then be cached.
*   **Cons:** The structure is more complex than a simple linked list. Accessing blocks in a very large file incurs the overhead of reading the intermediate indirect blocks.

## 7. Conclusion
File systems are the OS's preferred method for managing persistent data. They must be designed to balance the competing goals of performance, reliability, and security. Modern operating systems use a layered VFS architecture to support multiple, diverse file system designs. Key design aspects, such as the choice between a linked-list structure (FAT) and an indexed structure (inodes), have a profound and lasting impact on the capabilities and performance of the file system.
