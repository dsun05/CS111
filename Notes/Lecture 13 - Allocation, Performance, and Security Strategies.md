# File Systems: Allocation, Performance, and Security Strategies

### 1. Free Space and Allocation Management

File systems are dynamic; files are constantly created, deleted, and modified. These operations convert free disk blocks into used blocks and vice-versa. The operating system requires efficient and correct methods to manage this free space. This typically involves maintaining a list or structure of all unused disk blocks.

#### 1.1. Free Space Data Structures

The file system must keep track of which portions of its allocated storage are free. While this could be done with a traditional free list, similar to those used for variable-sized partitions in RAM, file systems typically allocate space in fixed-size blocks. This allows for simpler and more efficient data structures.

*   **Free List:** A linked list of pointers to free blocks. This is more suitable for variable-sized allocations.
*   **Bitmap (or Bit Vector):** A structure with a single bit for each block on the device. One value (e.g., 0) indicates the block is free, and the other (e.g., 1) indicates it is allocated. Searching a bitmap involves fast logical operations (like shifts and ORs on words of the map), making it significantly quicker than traversing pointers in a free list. Most modern file systems use bitmaps.

#### 1.2. Storage Technology Considerations

The choice of which free block to allocate is influenced by the underlying storage technology.

*   **Hard Disk Drives (HDDs):** Locality is critical. Placing blocks of a single file physically close to one another on the rotating platter minimizes seek time and rotational latency, improving read/write performance.
*   **Flash Drives (SSDs):**
    *   **Locality:** Physical locality is not a performance factor, as any block can be accessed at roughly the same speed.
    *   **Erasure:** Flash memory is a write-once medium at the block level. To rewrite a block, a larger containing area, called a *sector* or *erase block*, must first be erased. This makes it beneficial to organize data so that entire sectors can be freed up and erased at once.
    *   **Wear Leveling:** Each flash sector can only be erased a limited number of times. To prevent certain sectors from wearing out prematurely, the file system must distribute write and erase operations evenly across the entire drive. This is known as wear leveling.

#### 1.3. File Creation, Extension, and Deletion

##### 1.3.1. Creating a New File
Creating a new file involves two main steps: allocating a file control block and giving the file a name.

1.  **Allocate a File Control Block:**
    *   **UNIX-style systems:** The system searches the free inode list or bitmap for an unused inode. It takes the first available one (or a "good" one based on locality or other heuristics) to serve as the file's control block.
    *   **DOS (FAT) systems:** The system searches the parent directory for an unused directory entry. The directory entry itself serves as the file control block. If the directory is full, a new block may be allocated to extend it.
2.  **Initialize the File Control Block:** Information such as file type, ownership, and access permissions is written into the newly allocated inode or directory entry.
3.  **Name the File:** The file's name is written into a directory.
    *   **UNIX:** The name and a pointer to the file's inode number are added to a directory file.
    *   **DOS:** The name is part of the file control block (the directory entry).

##### 1.3.2. Extending a File
When an application writes new data to a file, requiring more space, the file system must allocate new blocks.

1.  **Find a Free Chunk:** Traverse the free list or bitmap to find a suitable free block.
2.  **Allocate the Chunk:** Remove the chosen block from the free list/bitmap, marking it as used.
3.  **Associate with the File:** Update the file's control block (e.g., an inode or FAT table) to point to the newly allocated block. For example, in a UNIX inode, a direct or indirect pointer is updated to the address of the new block. Generally, the directory entry does not need to be updated after the initial creation.

##### 1.3.3. Deleting a File
When a file is deleted, its data blocks and file control block must be returned to the free pool.

*   **UNIX-style Systems (Immediate Deallocation):**
    1.  The file system traverses all pointers in the file's inode (direct, single-indirect, double-indirect, etc.).
    2.  For each pointer, it follows it to the corresponding data block (or indirect block) and adds that block back to the free list/bitmap.
    3.  After all data and indirect blocks are freed, the inode itself is cleared and returned to the free inode list.
    4.  The directory entry containing the file's name is removed.
    This process is thorough but can incur significant overhead at the time of deletion.

| **Example: Deleting a UNIX File** |
| :--- |
| 1. The system accesses the file's inode. |
| 2. It follows the first direct pointer, frees the corresponding data block, and nulls the pointer in the inode. This is repeated for all direct blocks. |
| 3. It then moves to the single indirect block pointer. It reads the indirect block. |
| 4. It iterates through the pointers in the indirect block, freeing each data block they point to. |
| 5. Once all data blocks pointed to by the indirect block are free, the indirect block itself is freed. |
| 6. This process continues for double and triple indirect blocks if they are in use. |
| 7. Finally, the inode is marked as free. |

*   **DOS (FAT) Systems (Delayed Deallocation via Garbage Collection):**
    1.  **Deletion:** To delete a file, the system only changes the first byte of its name in the directory entry to a special character (e.g., null). This is a very fast operation. The data blocks and FAT chain are left untouched.
    2.  **Garbage Collection:** At a later time (e.g., when the system is idle or running low on space), a garbage collection utility runs.
        *   It scans all directories for entries marked as deleted.
        *   For each deleted entry, it finds the starting block number from the directory entry.
        *   It follows the chain of entries in the File Allocation Table (FAT), freeing each block in the chain by writing a zero into its FAT entry. This continues until the end-of-file marker is reached.

| **Example: DOS Garbage Collection** |
| :--- |
| 1. A file `myfile.txt` is removed. Its directory entry's first byte is set to `0`. |
| 2. The garbage collector later finds this entry. It notes the starting block is `3`. |
| 3. It goes to entry `3` in the FAT table. It sees the next block is `8`. It zeros out entry `3` and moves to entry `8`. |
| 4. It sees the next block is `6`. It zeros out entry `8` and moves to entry `6`. |
| 5. It sees the next block is `11`. It zeros out entry `6` and moves to entry `11`. |
| 6. It sees an end-of-file marker (`-1`). It zeros out entry `11`. The chain is now broken and all blocks (`3`, `8`, `6`, `11`) are marked as free. |

---

### 2. Performance Improvement Strategies

Due to the high latency of storage devices compared to RAM, file systems employ several strategies to improve performance.

#### 2.1. Transfer Size
The overhead for each I/O operation (DMA setup, interrupts) is high. Larger transfer units are more efficient because they amortize this fixed cost over more bytes.

*   **Allocation Units (Clusters/Chunks):** Instead of allocating single blocks, systems can allocate contiguous groups of blocks (e.g., 4 blocks, or 16KB). This allows for large, efficient DMA transfers.
*   **Trade-offs:**
    *   **Internal Fragmentation:** Using large, fixed-size chunks leads to wasted space if a file's size is not a multiple of the chunk size (e.g., a 1-byte file in a 32KB chunk).
    *   **External Fragmentation:** Using variable-sized chunks can lead to many small, non-contiguous free spaces over time, making it difficult to find large contiguous chunks for new files.

#### 2.2. Caching
Caching is the most important performance optimization. It involves keeping frequently accessed data in faster memory (RAM) to avoid slow device I/O.

##### 2.2.1. Read Caching
*   **Block I/O Cache:** When a block is read from the disk, it is stored in an in-memory cache. Before scheduling any read I/O, the file system first checks this cache. If the block is present (a cache hit), the data is served directly from RAM, which is orders of magnitude faster. This relies on the principle of locality (temporal and spatial).

*   **Read-Ahead (Prefetching):** When the file system detects a sequential access pattern (e.g., a process reading a file from beginning to end), it can proactively request subsequent blocks from the device before the application explicitly asks for them.
    *   **Benefit:** Reduces process wait time. When the application requests the next block, it is likely already in the cache, eliminating I/O latency.
    *   **Risks:** If the access pattern is not sequential, read-ahead wastes device bandwidth and pollutes the cache with unneeded blocks, potentially displacing more useful data.

##### 2.2.2. Write Caching (Delayed Writes)
*   **Write-Back Cache:** When an application writes to a file, the changes are made to the block's copy in the in-memory cache. The block is marked as "dirty." The actual write to the persistent storage device is deferred.
*   **Benefits:**
    1.  **Aggregation:** Multiple small writes to the same block are combined into a single, more efficient disk write.
    2.  **Write Elimination:** If data is rewritten multiple times or if a temporary file is created and deleted before the cache is flushed, the disk writes can be eliminated entirely.
    3.  **Improved Scheduling:** Accumulating a queue of write requests allows the disk scheduler to optimize the order of operations to minimize head movement (on an HDD).
    4.  **Application Responsiveness:** The application does not have to wait for the slow disk write to complete.

##### 2.2.3. Specialized Caches
In addition to a general-purpose block cache, systems can use specialized caches for better performance.

*   **Directory Caches:** Speed up the process of searching directories for file names.
*   **Inode Caches:** Keep inodes of frequently used or currently open files in memory to speed up access to file metadata.
*   These caches are more complex but can work better by matching the cache granularity to the specific data structures being used.

##### 2.2.4. Pinning Data in Caches
Some critical file system data may be **pinned** in memory, meaning it is not subject to normal cache replacement policies (like LRU). This ensures it remains in RAM for quick access.
*   **Example:** The inode of a file that a process currently has open is often pinned because it will likely be accessed repeatedly. Contents of a user's current working directory may also be pinned.

---

### 3. File System Security

In a multi-user environment, the file system must provide security to control who can access which files and in what manner.

#### 3.1. The Access Control Model
Security is typically enforced by the operating system whenever a process attempts to access a file via a system call. The most common model is **per-file, per-user access control**.

To decide whether to grant or deny access, the file system needs to answer three questions:
1.  **Who is requesting access?** The identity of the user is typically determined from the *process descriptor* of the process making the system call.
2.  **Which file is being accessed?** This is specified as a parameter (e.g., a pathname) in the system call.
3.  **What operation is being requested?** This is determined by the specific system call being made (e.g., `open`, `read`, `write`).

#### 3.2. Access Check Procedure
The check is typically performed when a file is first opened.

| **Example: Security Check on `open()`** |
| :--- |
| 1. A process belonging to user "Sue" makes an `open("foo", O_RDONLY)` system call. |
| 2. The OS code handling the call consults the process descriptor to confirm the user is "Sue." |
| 3. The file system locates the file descriptor (inode) for the file "foo" on disk and loads it into memory (if not already cached). |
| 4. It examines the access control information (permissions bits) within the inode. |
| 5. It checks if "Sue" is allowed to perform a "read" operation on this file. |
| 6. If access is allowed, the `open()` call succeeds, and the process receives a file handle. If not, the call fails. |

#### 3.3. Implications of the Model
*   **Check at `open()` Time:** In most systems, the security check is performed only at `open()` time. Once a file is successfully opened for a certain mode (e.g., read), subsequent `read()` calls by that process on that file handle are not re-checked against the file's on-disk permissions.
*   **Data Cannot Be "Taken Back":** If a user reads data from a file, that data is copied into their process's memory. If the file owner later revokes the user's read permission, the user still has the copy of the data they already read. The secret is out, and it cannot be recalled.
*   **Bypassing File System Abstractions:** File system security operates at the file abstraction level. If a privileged user can gain raw, block-level access to the storage device (e.g., by reading a special device file like `/dev/sda1`), they can bypass all file-level permissions and read any data on the disk. Therefore, securing these low-level access paths is critical for true file system security.