# File Systems – Naming and Reliability

## 1. File System Naming

File systems must provide a naming mechanism that is both user-friendly and efficiently translatable for the operating system's internal use. While the OS prefers simple numbers (like inode numbers) to identify files, humans and applications require more meaningful, typically string-based, names. An effective naming scheme must be user-friendly, allow for the organization of vast numbers of files, and be practically implementable within the file system's structure.

### 1.1. File Names and Binding

The core task of a file system's naming layer is to manage the mapping between human-readable names and the file system's internal descriptor structures (e.g., inodes).

*   **Binding**: The process of associating a name with a file. This typically occurs when a file is created, but names can also be changed (re-bound) later.
*   **Name Space**: This term has two common meanings:
    1.  The total collection of all file names that currently exist within a file system.
    2.  The entire set of all possible names that *could* be created by the naming mechanism.

### 1.2. Name Space Structure

There are two primary ways to structure a namespace:

1.  **Flat Name Spaces**: An early and simple approach where all names exist in a single collection or pool. In this model, every file must have a unique name within the entire file system.
2.  **Graph-Based Name Spaces**: The modern standard, used by systems like Linux, Windows, and macOS. This structure organizes files into a graph.
    *   **Strict Hierarchical Tree**: Each file or directory has exactly one parent, creating a single path from the top (root) to any given node.
    *   **General Directed Graph**: A more flexible structure where a file can have multiple parents, allowing for multiple paths to the same file. This occurs when a file can have more than one name.

A system can have a single, unified namespace that encompasses all storage devices, or it can treat each file system on each device as a separate, independent namespace.

### 1.3. Hierarchical Name Spaces and Directories

Modern systems use a hierarchical structure built upon a special type of file called a **directory**.

*   **Directory**: A file whose content is a list of references to other files. Each reference maps a name to a file's descriptor (e.g., a file name to an inode number). Directories are non-leaf nodes in the file system graph.
*   **Directory Tree**: Nested directories form a tree-like structure starting from a single **root** node. In Unix-like systems, the root is denoted by `/`; in Windows, it's `\`.
*   **Path Name**: A string that specifies the path from a starting point (like the root) through a sequence of directories to a target file.
    *   **Fully Qualified Name**: A path name that begins at the root directory, providing a complete and unambiguous path to a file. Example: `/user_3/dir_a/file_b`.
*   **Current Working Directory**: To simplify naming, each process has a "current directory." File names that do not start from the root are interpreted relative to this current directory.

#### 1.3.1. Example of a Rooted Directory Tree

The following diagram illustrates a simple file system hierarchy:

```
        root
       /  |  \
      /   |   \
  user_1 user_2 user_3
   / \          / \
  /   \        /   \
file_a dir_a  file_b file_c dir_a
         |                |
       file_a           file_b
```

*   The file at `/user_1/file_a` is distinct from the file at `/user_1/dir_a/file_a`.
*   The directory `/user_1/dir_a` is distinct from `/user_3/dir_a`.

#### 1.3.2. Directories as Special Files

To maintain the integrity of the file system's structure, directories are treated as a special file type.

*   **Reading**: User applications are generally allowed to read the contents of a directory (e.g., to list its files).
*   **Writing**: Direct writing to a directory file by user applications is typically prohibited. Modifying a directory's contents (creating, deleting, or renaming files) must be done through specific system calls (`create()`, `unlink()`, etc.). This prevents corruption of the file system's namespace structure.

#### 1.3.3. Traversing the Directory Tree

To navigate the hierarchy, special directory entries are used:

*   **`.` (dot)**: A standard entry in every directory that refers to the directory itself.
*   **`..` (dot-dot)**: A standard entry in every directory (except the root) that refers to its parent directory. This allows for upward traversal in the hierarchy.

### 1.4. Hard Links (Unix/Linux)

In Unix-like systems, a file can have multiple names. The underlying file is uniquely identified by its **inode**, a data structure containing all metadata except the name.

*   **Hard Link**: The association of a file name with an inode. A directory entry in these systems consists of a name and a pointer to an inode.
*   **Multiple Names**: A single file (one inode) can have multiple directory entries pointing to it, potentially from different directories. These are all hard links. All names are equal; there is no "true" or "primary" name.
*   **Metadata**: All metadata (permissions, owner, block pointers) is stored in the inode. Therefore, all hard links to a file share the same permissions and point to the same data.
*   **De-allocation and Link Count**: To manage de-allocation, the inode contains a **link count**, which is a reference counter of how many hard links point to it.
    *   When a link is created, the count is incremented.
    *   When a link is removed (e.g., with the `rm` command), the count is decremented.
    *   The file's data and inode are only de-allocated and returned to the free list when the link count drops to zero.

#### 1.4.1. Hard Link Example

Consider a file with inode #29. Two separate directory entries point to it:
1.  In directory `/user_1` (inode #9), the name `file_a` points to inode #29.
2.  In directory `/user_3/dir_a` (inode #194), the name `file_c` also points to inode #29.

In this case, inode #29 would have a link count of 2. Deleting `/user_1/file_a` would decrement the count to 1. The file would still be accessible via `/user_3/dir_a/file_c`. Only after deleting the second link would the file's data be freed.

| Directory (`/user_1`, inode #9) | | Directory (`/user_3/dir_a`, inode #194) | | File (inode #29) |
| :--- | :--- | :--- | :--- | :-- |
| Name | Inode # | Name | Inode # | **Link Count = 2** |
| `file_a` | 29 | `file_c` | 29 | Data blocks... |

### 1.5. Symbolic Links (Soft Links)

Symbolic links provide another way to create multiple names for a file, functioning as aliases or shortcuts.

*   **Definition**: A symbolic link is a special type of file whose content is simply the path name of another file.
*   **Mechanism**: When the file system accesses a symbolic link, it reads the path contained within it and then resolves that path to find the target file.
*   **Contrast with Hard Links**:
    *   **No Inode Reference**: A symbolic link does not point directly to the target's inode and does not affect the target's link count.
    *   **Dangling Links**: If the target file is deleted or moved, the symbolic link is not automatically updated or removed. It becomes a "dangling" or "broken" link, and attempting to access it will result in an error (analogous to a 404 error on the web).
    *   **Spanning File Systems**: Symbolic links can point to files on different file systems, which hard links cannot do (as inode numbers are only unique within a single file system).

## 2. File System Reliability

File system reliability ensures that data is stored correctly and consistently, even in the face of system crashes or hardware failures.

### 2.1. The Core Reliability Problem

High-level file system operations are not atomic at the hardware level. For example, creating a new block in a file requires at least three separate disk writes:
1.  Writing the new data to a data block.
2.  Updating the file's inode to point to the new data block.
3.  Updating the free-space bitmap or list to mark the block as allocated.

If a system crash occurs after one or two of these writes but before the third, the file system is left in an **inconsistent state**. For instance, if the inode is updated but the free list is not, the same block could be allocated to two different files, leading to data corruption.

### 2.2. Approaches to Reliability

#### 2.2.1. Ordered Writes

A basic approach is to carefully order the sequence of dependent writes to minimize the damage of an inconsistent state.
*   **Rule 1: Write data before metadata.** Write data to a block before updating the inode to point to it. This avoids creating pointers to uninitialized or garbage data. The worst-case outcome of a crash is an allocated but unreferenced block (leaked space), which is less severe than data corruption.
*   **Rule 2: Perform deallocations before allocations.** When moving data, first update the inode to deallocate the old block, and only then update the free list to make it available. This avoids a state where both the file and the free list claim the same block.

While helpful, ordered writes reduce I/O performance by preventing optimizations like write consolidation and can be subverted by modern disk controllers that reorder write requests.

#### 2.2.2. Audit and Repair (`fsck`)

The traditional solution was to run a file system check utility (like `fsck` in Unix) after a crash. This tool scans the entire disk, cross-references all structures (inodes, directories, free lists), finds inconsistencies, and attempts to repair them. For modern multi-terabyte drives, this process is impractically slow, potentially taking many hours.

#### 2.2.3. Journaling File Systems

Journaling is a modern, efficient technique for ensuring crash consistency, borrowed from database systems.
*   **Journal**: A special, circular, append-only log located in a dedicated area of the disk.
*   **Mechanism**:
    1.  **Log the Transaction**: Before performing any disk writes, the file system first writes a log entry to the journal describing all the changes it intends to make (e.g., "write this data to block X, update inode Y, modify free list Z").
    2.  **Commit**: Once the entire transaction is safely written to the journal, the operation is considered "committed" and safe from crashes. The application can be notified of success at this point.
    3.  **Checkpoint**: In the background, the file system copies the changes from the journal to their final locations on the disk.
    4.  **Free**: Once the changes are applied to their final locations, the corresponding entry in the journal can be marked as complete and its space reclaimed.
*   **Recovery**: After a crash, the recovery process only needs to read the small journal. It scans for any transactions that were committed but not yet fully checkpointed and simply replays them. This process is very fast. These replay operations are **idempotent**, meaning they can be performed multiple times with the same result, making recovery safe even if it's unclear whether an operation was partially completed.

*   **Metadata-Only Journaling**: A common optimization where only changes to metadata (inodes, directories, free lists) are logged in the journal, not the file data itself. The data is written to its final location first. This saves journal space and bandwidth, as metadata is small but its integrity is critical, while file data is often large.

#### 2.2.4. Log-Structured File Systems (LFS)

LFS takes the journaling concept to its logical conclusion: the entire file system *is* the log.
*   **Mechanism**: All writes—both data and metadata—are sequentially appended to the end of a single, disk-spanning log. Data is never overwritten in place.
*   **Redirect-on-Write**: When a file is modified, the new data and a new copy of its updated inode are written to the end of the log. Pointers are then updated to refer to these new locations.
*   **Inode Map**: Because inodes are no longer in fixed locations, an `inode map` is required to track the current disk location of the latest version of each inode. This map is also periodically written into the log.
*   **Advantages**:
    *   All writes are converted into large, sequential writes, which is highly efficient, especially for rotational disks.
    *   It aligns perfectly with the nature of flash storage, which cannot overwrite data in place and performs better with sequential writes.
    *   Old versions of data and metadata remain in the log (until cleaned), making features like file snapshots nearly free to implement.
*   **Disadvantages**:
    *   It requires a background **garbage collection** or "cleaning" process to run, which finds obsolete blocks in the log, compacts the live data, and frees up large segments for future writes.
    *   Read performance for fragmented files can be poor if their blocks are scattered across the log.