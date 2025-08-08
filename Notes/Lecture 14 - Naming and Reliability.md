# Lecture 14: File Systems – Naming and Reliability

## 1.0 File System Naming

### 1.1 The Need for Naming Systems
*   **OS Preference vs. User Needs:**
    *   The Operating System (OS) and its internal code prefer to work with simple, fixed-size numbers as file handles. Numbers are efficient, easy to manage in code, and not prone to user error like misspellings.
    *   Users and application programs find numbers unusable and require more meaningful, human-readable names, typically strings, to identify, organize, and interact with files.
*   **Goals of a Naming System:**
    *   **User-friendly:** Names must be easy for people to understand and remember.
    *   **Organizational:** The system must allow for the easy organization of large numbers of files into logical groups.
    *   **Realizable:** The naming scheme must be practically and efficiently implementable within the file system's code and on physical storage devices.

***
### 1.2 File Names and Binding
*   **Name-to-File Mapping (Binding):** Binding is the process of creating an association between a human-readable name and a file's underlying representation (e.g., its descriptor structure or inode number). This typically occurs when a file is created but can also happen when a file is renamed.
*   **Name Space:** This term has two common meanings:
    1.  The total collection of all file names that currently exist on a given file system or machine.
    2.  The set of all possible names that *could* be created by the naming mechanism, which is a much larger, theoretical set.

***
### 1.3 Name Space Structures
*   **Flat Name Spaces:** An early, simple structure where all file names exist in a single collection or pool. In this model, every file must have a globally unique name, as there is no hierarchy to provide context.
*   **Graph-Based Name Spaces:** A modern approach where names are organized in a graphical structure.
    *   **Hierarchical Tree:** A strict parent-child structure where each file or directory (node) has only one parent. There is exactly one path from the root to any given node.
    *   **General Directed Graph:** A more flexible structure where a node (a file) can have multiple parents, meaning there can be multiple distinct paths to reach the same file. This is created when files are allowed to have multiple names.
*   **Scope:** A design choice regarding how many name spaces exist on a machine. A system can have a **single, unified name space** that encompasses all files on all storage devices, or it can have **multiple independent name spaces**, one for each disk or partition.
*   **Key Design Issues:**
    *   **How many files can have the same name?** In a flat name space, only one. In a hierarchical name space, one file with a given name is allowed per directory.
    *   **How many different names can one file have?** Options include: a single "true name"; one "true name" plus aliases; or an arbitrary number of equally valid names.
    *   **Do different names for the same file have different characteristics or permissions?** For example, does deleting one name affect the others? Do different names grant different access rights?

***
### 1.4 Hierarchical Name Spaces and Directories
*   **Organization:** Hierarchical name spaces are typically organized using directories. A directory is a special type of file that contains references (name-to-descriptor mappings) to other files and other directories.
*   **Structure:**
    *   Nested directories form a tree-like structure, with directories acting as non-leaf nodes and regular files as leaf nodes.
    *   The entire structure originates from a single top-level directory called the `root`.
    *   A `path` is the sequence of directories traversed from a starting point to a specific file.
    *   A `fully qualified name` (or absolute path) is a path that starts from the `root` directory, providing a complete and unambiguous location for a file.
*   **Properties of Directories as Special Files:**
    *   **Fundamental Identity:** Directories are files, but a special type whose purpose is to implement the name space hierarchy.
    *   **Content and Structure:** A directory file's data blocks do not contain arbitrary user data. Instead, they contain a highly structured list of **directory entries**. Each entry typically consists of a file name and a pointer to that file's descriptor (e.g., an inode number).
    *   **Restricted Operations for Integrity:**
        *   The integrity of the file system's structure depends entirely on the correctness of the directory files.
        *   **Writes:** To prevent corruption of the name space, users and applications are **not allowed to write directly** to directory files using standard `write` system calls. Modifications are performed indirectly through dedicated system calls (`create`, `unlink`, `mkdir`, `rename`) that ensure the directory's internal structure remains valid.
        *   **Reads:** While applications can read directories to get information (e.g., list files), this is typically done through library functions (`readdir`) that parse the structured data, rather than reading raw bytes.
    *   **Special Directory Entries:** Directories contain two special, self-referential entries:
        *   `.` **(dot):** A hard link that points to the inode of the directory itself. This provides a convenient way for a process to reference its current working directory.
        *   `..` **(dot-dot):** A hard link that points to the inode of the **parent directory**. This entry is what enables upward traversal in the file system hierarchy. In the root directory, which has no parent, `..` points back to the root's own inode.
*   **Directory Traversal:**
    *   Paths are navigated using a `delimiter` character (e.g., `/` in Unix/Linux, `\` in Windows) to separate the name components.
    *   Special entries are included in directories to facilitate navigation:
        *   `..` (dot-dot) is a standard entry that refers to the parent directory, allowing for upward movement in the hierarchy.
        *   `.` (dot) is a standard entry that refers to the directory itself.
*   **Example: A Rooted Directory Tree**
    This diagram illustrates a hierarchical file system structure.

    ```
    / (root)
    |
    +-- user_1/
    |   |
    |   +-- file_a       (path: /user_1/file_a)
    |   |
    |   `-- dir_a/
    |       |
    |       `-- file_a   (path: /user_1/dir_a/file_a)
    |
    +-- user_2/
    |   |
    |   `-- file_b       (path: /user_2/file_b)
    |
    `-- user_3/
        |
        +-- file_c       (path: /user_3/file_c)
        |
        `-- dir_a/
            |
            `-- file_b   (path: /user_3/dir_a/file_b)
    ```

***
### 1.5 File Names vs. Path Names: Two Fundamental Models
*   **Model 1: The "True Name" System (e.g., DOS/FAT)**
    *   **Concept:** A file is considered to have a single, intrinsic, canonical name that is part of its identity.
    *   **Mechanism:** The local name of the file is stored within its directory entry. The file's one "true name" is the unique, fully qualified path to that specific directory entry.
    *   **Implication:** This model creates a rigid, one-to-one relationship between a name and a file. A file has exactly one name, and a name points to exactly one file. There is no native concept of multiple first-class names for a single file.
*   **Model 2: The "Path as Name" System (e.g., Unix/Linux)**
    *   **Concept:** This model makes a fundamental distinction between a file's identity and its name. A file has no inherent name stored within its own metadata (the inode). A file's identity is simply its low-level descriptor—the inode.
    *   **Mechanism:** A "name" is not a property of the file itself, but is instead an entry in a directory that points to the file's inode. Therefore, a name is effectively just a human-readable pointer that exists as one component of a path.
    *   **Implication:** Because the name is separate from the file's identity, any number of directory entries in any number of directories can point to the same inode. This directly and naturally leads to the concept of **multiple names for a single file**, where each name is just a different path that resolves to the same inode. This makes **hard links** a core, logical feature of the file system design, rather than an add-on.

***
### 1.6 Unix File Naming and Links
*   **Hard Links:**
    *   A hard link is a directory entry that directly maps a human-readable name to a specific inode number. A Unix directory entry consists simply of a name and an inode pointer.
    *   A single file (represented by one inode) can have multiple hard links pointing to it from various directories.
    *   All hard links are equal; there is no primary or "true" name. Each is a direct, first-class reference to the file's inode. All file metadata (permissions, owner, block pointers) is stored in the inode, not the directory entry.
    *   **De-allocation:** An inode contains a **link count**, which is a reference counter tracking how many hard links point to it. The file's data blocks and its inode are only de-allocated and returned to the free lists when this link count drops to zero (i.e., the last name for the file is deleted).
*   **Example: Hard Links, Directories, and Files**

    This diagram shows two directory entries pointing to the same file inode, creating a hard link.

    ```
	inode #1 (Root Directory, /)
	+-----------+----------+
	| File Name | Inode #  |
	+-----------+----------+
	| .         | 1        |
	| ..        | 1        |
	| user_1    | 9   -----+----> inode #9 (Directory, /user_1)
	| user_3    | 114 ----\|    +-----------+----------+
	+-----------+---------|+    | File Name | Inode #  |
						  |     +-----------+----------+
						  |     | .         | 9        |
						  |     | ..        | 1        |
						  |     | file_a   | 29   -----+--\
						  |      +-----------+---------+  |
						  V                               |
			 inode #114 (Directory, /user_3)              |
			 +-----------+----------+                     |
			 | File Name | Inode #  |                     |
			 +-----------+----------+                     |
			 | .         | 114      |                     |
			 | ..        | 1        |                     +----> inode #29 (File)
			 | file_c    | 29  -----+---------------------/     +----------------+
			 +-----------+----------+                           | Type: File     |
																| Link Count: 2  |
																| ... (other     |
																| metadata)      |
																+----------------+
    ```

*   **Symbolic Links (Soft Links):**
    *   A symbolic link is a special type of file whose data content is simply a text string representing a path to another file.
    *   It is an indirect reference, or alias. When the OS accesses a symbolic link, it reads the path stored within it and then resolves that path to find the target file. It is similar in concept to a shortcut in Windows or a URL on the web.
    *   Creating a symbolic link does not affect the target file's link count.
    *   The link can become "dangling" or broken if the target file is moved or deleted, as the path stored in the link will no longer point to a valid file. The system will return a "file not found" error.
*   **Example: Symbolic Links, Files, and Directories**

    This diagram shows an indirect link via a symbolic link file.

    ```
	STEP 1: Follow path /user_3/file_c
	+----------------------------+
	| inode #1 (Root Dir, /)     |
	| +-----------+------------+ |
	| | user_3    | inode #114 |->---------+
	| +-----------+------------+ |         |
	+----------------------------+         |
										   V
							  +----------------------------+
							  | inode #114 (Dir, /user_3)  |
							  | +-----------+------------+ |
							  | | file_c    | inode #46  |->---------+
							  | +-----------+------------+ |         |
							  +----------------------------+         |
																	 V
														 +--------------------------+
														 | inode #46 (Symlink File) |
														 | Data: "/user_1/file_a"   |
														 +--------------------------+

	STEP 2: OS reads path from symlink and resolves it from the root
	+----------------------------+
	| inode #1 (Root Dir, /)     |
	| +-----------+------------+ |
	| | user_1    | inode #9   |->---------+
	| +-----------+------------+ |         |
	+----------------------------+         V
							  +----------------------------+
							  | inode #9 (Dir, /user_1)    |
							  | +-----------+------------+ |
							  | | file_a    | inode #29  |->---------+
							  | +-----------+------------+ |         |
							  +----------------------------+         V
																  +-----------------+
																  | inode #29 (File)|
																  | Link Count: 1   |
																  +-----------------+
    ```

***
## 2.0 File System Reliability

### 2.1 The Core Reliability Problem
*   A single high-level operation (e.g., creating or extending a file) often requires multiple, separate low-level write operations on the disk. For example, allocating a new block requires updating:
    1.  The data block itself with the new file content.
    2.  The file's inode to add a pointer to the new data block.
    3.  The free-space bitmap or list to mark the block as allocated and no longer free.
*   A system crash that occurs between these low-level writes can leave the file system in an **inconsistent state**, where the metadata no longer accurately reflects the state of the data, leading to corruption.
*   **Types of File System Corruption:**
    *   **Improperly Shared Data:** The most dangerous state, where two different files' inodes point to the same data block, causing one file's writes to corrupt the other's data.
    *   **Lost Free Space:** A data block is not pointed to by any file but is also not marked as free in the free-space list, causing a permanent space leak until fixed.
    *   **Dangling Pointers:** An inode or directory entry points to a resource that has been deallocated or now contains garbage data.
    *   **Unfindable Files:** A valid inode and its data blocks exist on disk, but no directory entry points to the inode, making the file inaccessible.
    *   **Corrupted Directories/Inodes:** Physical damage or incorrect writes to metadata can make files or entire directory trees inaccessible.
*   **Example: Worst-Case Scenario**
    This diagram shows how a crash after an inode write but before a free-list write leads to a shared block.

    ```
    INITIAL STATE:
      - File A needs a new block.
      - Block #512 is marked as FREE in the on-disk free list.
	  - Free-list write-back is deferred

    STEP 1: The OS updates File A's inode in memory and writes it to disk.
      - File A's on-disk inode now points to Block #512.
      - The OS writes the new data to Block #512 on disk.

    >>> SYSTEM CRASH <<<

    STEP 2: The system reboots. The on-disk free list was never updated.
      - OS reads the free list: Block #512 is still marked as FREE.

    STEP 3: A new process requests a block for File B.
      - The OS consults the (stale) free list and allocates Block #512 to File B.
      - File B's on-disk inode is now also pointing to Block #512.

    RESULT: DATA CORRUPTION
      - Two different files point to the same physical block on disk.
      - Any write to File A will overwrite File B's data, and vice-versa.
    ```

***
### 2.2 The Performance vs. Reliability Trade-off
*   **Application Expectations:** Users and applications operate under the assumption that when a `write` system call returns, the data is "safe" and will persist through a system crash.
*   **The Synchronous ("Safe") Writing Alternative:**
    *   To truly guarantee safety, the OS could perform **synchronous writes**. This means the system call does not return control to the application until all associated physical writes to the disk have been confirmed as complete.
*   **The Prohibitive Cost of Blocking:**
    *   **Multiple Slow Operations:** A single logical write requires waiting for *multiple* physical disk writes (e.g., for the data, inode, and free list). Disk I/O is inherently slow, measured in milliseconds.
    *   **Stalled Application:** While waiting for the disk, the application is **blocked**. It is completely stalled and cannot proceed with any other computation. This creates a severe, user-visible performance bottleneck.
    *   **Constant, Certain Penalty:** This blocking cost is not an occasional event; it is a *certain* and *constant* performance penalty paid on *every single write operation*.
*   **The Trade-off Decision:**
    *   System designers must weigh a **certain, massive performance penalty** on millions of normal write operations against the benefit of protecting against a **rare, occasional event** (a system crash).
    *   The universal conclusion is that forcing every write to be synchronous is an unacceptable performance trade-off.
*   **The Justification for Buffered (Deferred) Writes:**
    *   To solve this, the OS uses **buffered writes**. It accepts a small risk of inconsistency during a rare crash in exchange for enormous performance gains.
    *   The OS "lies" to the application by immediately returning from the system call after placing the data in a fast RAM buffer. This uncouples the application's speed from the disk's speed, creating a responsive user experience. This "lie" creates the window of vulnerability that reliability strategies like journaling are designed to close.

***
### 2.3 Strategies for Ensuring Reliability

#### 2.3.1. Strategy 1: Ordered Writes

*   **Concept:** This strategy attempts to improve reliability by carefully sequencing the order of dependent low-level write operations. The goal is to ensure that if a crash occurs mid-sequence, the resulting file system state is non-catastrophic (e.g., resulting in lost space rather than corrupted data).
*   **Key Principles:**
    1.  **Write data before pointers:** Always write the file's data blocks to disk *before* writing the inode that points to them. If a crash occurs after the data is written but before the pointer is updated, the result is unreferenced "orphan" data, which is harmless and can be cleaned up later. The reverse order—writing a pointer to a block that contains garbage data—is much more dangerous.
    2.  **Perform deallocations before allocations:** When reassigning a block, first write the metadata change that frees the block from its old owner (e.g., nullifying the pointer in the old inode). *Then*, write the change that allocates it to the new owner. A crash between these steps results in a "lost" block (not owned by anyone and not on the free list), which is safer than an "improperly shared" block where two files point to the same data.
*   **Drawbacks:**
    *   **Performance Penalty:** This approach severely reduces I/O performance by preventing the file system from using common optimizations like accumulating nearby writes or consolidating multiple updates to the same block into a single write.
    *   **Not a Complete Solution:** It does not eliminate the problem of incomplete multi-part writes, it merely chooses a less damaging failure state.
    *   **Modern Hardware Issues:** The strategy is not fully effective on modern storage devices that may reorder write requests in their internal hardware queues for their own performance reasons, thus violating the OS's intended sequence.

***
#### 2.3.2. Strategy 2: Audit and Repair (e.g., `fsck`)

*   **Concept:** This is a post-crash recovery strategy. After an unclean shutdown, a dedicated utility program (like `fsck` in Unix or `CHKDSK` in Windows) runs at boot time. This program performs a full, exhaustive scan of the *entire* file system's on-disk metadata structures.
*   **Process:** The utility cross-references directory entries, inodes, and free-space maps to find and repair inconsistencies. For example, it can find allocated blocks that are not referenced by any file and return them to the free list, or fix incorrect link counts in inodes.
*   **Drawbacks:**
    *   **Obsolete due to Speed:** This approach is now considered impractical for primary recovery because it is prohibitively slow. The time required to read and analyze all metadata on a modern multi-terabyte drive is unacceptably long for a system reboot.
    *   **Illustrative Calculation:** Checking a 2TB file system at a typical 100MB/second disk transfer rate would take **5.5 hours** for the read phase alone, before any computational analysis or repair writes.

***
#### 2.3.3. Strategy 3: Journaling (Data and Metadata)

*   **Concept:** Journaling, also known as write-ahead logging, ensures the atomicity of file system updates. Before modifying the main file system structures, the OS first writes a description of the intended changes (a transaction) to a dedicated, contiguous log area called a **journal**. Only after the transaction is safely committed to the journal is the operation considered persistent.
*   **The Journal as a Physical Entity:** The journal is typically a **circular buffer** of a fixed, relatively small size, residing in a dedicated area on the storage device. To manage the circular buffer and distinguish new entries from old ones, transactions are marked with **timestamps** or sequence numbers. For maximum speed, the journal could potentially be placed in ultra-fast **NVRAM**.
*   **Performance Characteristics:**
    *   **Normal Operation:** Journal writes are extremely fast because they are always **sequential**, avoiding slow disk seeks and enabling efficient **DMA** transfers. The application is blocked only for the short time it takes to perform this fast write to the journal. The much slower, random-access writes to the final data locations are **decoupled** and scheduled later for optimal performance.
    *   **Recovery Process:** Recovery is very fast because the OS only needs to read the **small journal**, not the entire file system. It performs a single, large sequential read of the journal into RAM, does all analysis at CPU speed, and then performs only the few writes needed to complete any unfinished transactions.
*   **Recovery and Idempotency:** After a crash, the OS replays any transactions from the journal that were not fully completed. This is safe because the logged operations are **idempotent**—re-running them multiple times has the same effect as running them once, preventing further corruption.

	```
	Operation: Write two new pages of data (BLUE) to file /a/foo, replacing old data (RED).

	INITIAL STATE:
	  - Main Data Pages: [ RED | RED ]
	  - Journal Space:   [ ...empty... ]

	STEP 1: Write transaction to the journal.
	  - The OS performs one fast, sequential write to the journal containing the entire transaction.
	  - Journal Space:   [ START | metadata | BLUE_DATA | BLUE_DATA | END ]
	  - The `write` system call returns to the application NOW. The operation is "safe."

	STEP 2: Checkpoint the transaction to its final location (later, in the background).
	  - The OS copies the data from the journal to the main file system.
	  - Main Data Pages: [ BLUE | BLUE ]

	STEP 3: Clean up the journal.
	  - Once the data is in its final location, the journal entry is marked as complete and the space can be reused.
	  - Journal Space:   [ ...empty... ]
	```

***
#### 2.3.4. Strategy 3a: Meta-Data Only Journaling

*   **Concept:** A common and highly effective optimization of the journaling strategy where only changes to file system **metadata** (such as inodes, directory entries, and free-space bitmaps) are written to the journal. The actual file **data** is written directly to its final location on disk.
*   **Rationale:** This approach is taken for clear performance reasons:
    *   **Why journal metadata?** It is small, its write patterns are often random (making direct writes inefficient), and its structural integrity is absolutely critical to the health of the entire file system.
    *   **Why NOT journal data?** It is often large and written sequentially (already I/O efficient), and logging it would quickly consume the journal's limited bandwidth, effectively creating a "double write" penalty for all user data.
*   **Process:** This method carefully orders its operations to maintain consistency.
    1.  **Write data first:** The new data is written directly to a *new, unused* location in the main file system. The old version of the data is left untouched for now, ensuring a consistent version of the file is always available on disk.
    2.  **Journal metadata only:** A transaction containing *only* the metadata update (e.g., the inode change that points to the new block locations) is written to the journal. Once this fast, sequential write is complete, the `write` call can return to the application.
    3.  **Checkpoint and clean up:** At a later time, the file system applies the metadata change from the journal to the main on-disk metadata structures (the "checkpoint"). Only after the main inode is pointing to the new data are the old data blocks finally deallocated and returned to the free list.
*   **Diagram of Metadata-Only Journaling Process:**

    ```
    Operation: Overwrite two pages of data in file /a/foo.

    INITIAL STATE:
      - Main File System: File's inode points to [ OLD_DATA | OLD_DATA ]
      - Journal: [ ...empty... ]

    STEP 1: Write data to a NEW location on disk.
      - The old data blocks remain untouched.
      - Main File System: New blocks [ NEW_DATA | NEW_DATA ] are written.
      - The file's inode still points to the OLD data.

    STEP 2: Write ONLY the metadata change to the journal.
      - Journal: [ START | "Inode now points to NEW_DATA blocks" | END ]
      - The write() call returns to the application NOW. The operation is safe.

    STEP 3: Checkpoint the metadata (later, in the background).
      - The OS copies the metadata change from the journal to the main inode on disk.
      - Main File System: File's inode now points to [ NEW_DATA | NEW_DATA ]

    STEP 4: Deallocate the old data blocks.
      - Main File System: Blocks containing [ OLD_DATA | OLD_DATA ] are added to the free list.
    ```

***
#### 2.3.5. Strategy 4: Log-Structured File Systems (LFS)

*   **Concept:** In this radical design, the entire file system *is* the log. Data and metadata are never overwritten. Instead, all updates are bundled together and written sequentially to the head of a single, unified log that spans the entire disk using a technique called **Redirect-on-Write**.
*   **Redirect-on-Write (RoW):** This is a storage technique where, instead of modifying existing data "in-place," the new, updated version of the data is written to a **new, previously unused physical location**. After this new write is complete, the metadata pointer (e.g., in an inode or an inode map) that previously referenced the old data is updated (or "redirected") to point to the new location. This process leaves the original version of the data untouched, making it effectively immutable and enabling features like efficient snapshots.
*   **Modern Context and Applications:** LFS principles have become a dominant architecture in modern computing, especially for:
    *   **Flash File Systems:** The Redirect-on-Write model is a perfect match for flash memory, which cannot be efficiently overwritten in place and performs best with sequential writes to pre-erased blocks.
    *   **Key/Value Stores:** Many database and storage systems use LFS-like structures internally for high write throughput.
*   **Core Navigational Mechanics:**
    *   **Data Layout:** Large, sequential application writes result in data being laid out contiguously in the log. Small, random updates cause a file's data blocks to become fragmented and scattered throughout the log.
    *   **The Inode Map:** Since inodes are also written to the log and their locations change, a persistent index called an **inode map** is required to map a stable inode number to its latest physical location in the log. This map is itself periodically appended to the log to save its state.
*   **Recovery and Snapshots:**
    *   **LFS Recovery Process:** Recovery is fast and begins by finding the latest persistent version of the inode map. From that checkpoint, the system simply replays all subsequent log updates to bring the file system to its most recent, consistent state.
    *   **Immutability and Cheap Snapshots:** Because old data and inodes are not overwritten, they remain in the log. A snapshot can be created nearly instantly by simply preserving a pointer to an **older version of the inode map**. This makes the entire file system state from that point in time accessible without duplicating any data.
*   **Management and Garbage Collection: The Price of LFS**
    *   **The Problem: Log Fragmentation:** As the file system is used, the log becomes fragmented with a mix of valid, "live" data blocks and old, "dead" data blocks from files that were deleted or overwritten. Without a mechanism to reclaim the space occupied by dead blocks, the file system would quickly run out of space.
    *   **The Solution: The Cleaner / Garbage Collector:**
        *   LFS requires a background process, often called a **cleaner** or **garbage collector**, that runs during idle periods.
        *   This process reads several fragmented log segments into memory.
        *   It uses the inode map to identify which blocks within those segments are still "live" (i.e., are part of the current version of a file).
        *   It copies *only* the live blocks into a new, compacted segment.
        *   This new, clean segment is then written sequentially to the head of the log.
        *   The old segments, which now contain only dead or evacuated data, are marked as clean and can be erased and reused for future writes.
    *   **Overhead and Complexity:** This cleaning process is the primary drawback of LFS. It introduces significant I/O overhead (a "read-copy-write" cycle) that consumes disk bandwidth that could otherwise be used for new application writes. The efficiency and scheduling of the garbage collector are critical to the overall performance of the system.
    *   **Management Cost:** The system must constantly inventory and manage all old versions of data and metadata to determine which blocks are live and which can be recycled. This tracking adds significant management overhead.
*   **Illustrative Example: The "One Thing Leads to Another" Problem:**

    This multi-step diagram shows the cascading dependencies of a single write.

    ```
    Operation: Overwrite data in file /a/foo.

    STEP 1: Write new data to the head of the log.
      - [ NEW_DATA ] is written to the log. The log head moves forward.
      - PROBLEM: How do we find this new data? The existing inode for /a/foo
        still points to the [ OLD_DATA ] location in the log.

    STEP 2: Write a new inode to the log.
      - A [ NEW_INODE ] is created. It points to the [ NEW_DATA ].
      - This [ NEW_INODE ] is written to the head of the log. The log head moves.
      - PROBLEM: How do we find this new inode? The directory entry for "foo"
        inside the "/a" directory still points to the [ OLD_INODE ].

    STEP 3: Write a new directory block to the log.
      - A [ NEW_DIR_BLOCK ] for "/a" is created. It contains an entry for "foo"
        that points to the [ NEW_INODE ].
      - This is written to the head of the log. The log head moves.
      - PROBLEM: This creates a chain reaction up to the root. More importantly,
        how does the system find the location of ANY inode now?

    STEP 4: The Inode Map is the solution, but creates a new problem.
      - An Inode Map keeps track of the latest location of every inode.
      - To complete our write, we update the map entry for /a/foo's inode to point
        to the [ NEW_INODE ]'s location.
      - FINAL PROBLEM: This inode map must be persistent. So it must also be
        written to the log. This creates a recursive challenge: how do you find
        the pieces of the inode map itself? (This is solved with a fixed-location
        checkpoint area that points to the latest map).
    ```
	
***
## 3.0 Conclusion
*   File systems must balance user-friendly naming schemes with efficient internal management.
*   The hidden, underlying mechanisms of a file system are critical for achieving both high performance and robust reliability.
*   Modern approaches like journaling and log-structured designs are essential for managing the consistency and performance trade-offs on large, fast storage devices.
*   **The Power of Abstraction:** All of these complex and varied internal strategies (Journaling, LFS, Ordered Writes) are completely hidden from the user and application developer beneath a stable, high-level API (`open`, `read`, `write`). Application code remains the same, regardless of the sophisticated reliability mechanisms being used by the underlying file system.
