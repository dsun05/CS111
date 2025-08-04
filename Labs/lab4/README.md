# Hash Hash Hash
This lab contains two implementations of a hash table to allow for safe use concurrently. 'v1' is a naive and slow implementation, which is purely focused on correctness. This performs much slower than the base hash table, which is not concurrent safe and single threaded. 'v2' is an implementation that is both correct and performant, performing as good as the base/(num cores-1). 

UID: 806249571.

## Building
```shell
make
```

## Running
```shell
./hash-table -tester -t 8 -s 50000

Generation: 27,273 usec
Hash table base: 254,603 usec
  - 0 missing
Hash table v1: 879,306 usec
  - 0 missing
Hash table v2: 98,877 usec
  - 3 missing
```

## First Implementation
I added a global lock to the `hash_table` struct which is initialized when the hash table is created. When `hash_table_v1_add_entry` is called, a lock is acquired at the beginning and held for the duration of the operation. So, when any thread attempts to add an entry, it must first acquire the lock, which means that at most one thread can be actively modifying the table at a time. This trivially protects the `hash_table_v1_add_entry` function from race conditions, as no two threads can add an entry at the same time. The lock is then destroyed when the hash table itself is destroyed. This way, there is one lock per hash table, so all calls to `hash_table_v1_add_entry` for a specific table will go through that table's lock.

### Performance
```shell
Generation: 27,273 usec

Hash table base: 254,603 usec

  - 0 missing

Hash table v1: 879,306 usec

  - 0 missing

Hash table v2: 98,877 usec

  - 3 missing
```
Version 1 is slower than the base version due to multiple sources of overhead. First, it incurs overhead from the nature of threading itself. Creating a thread requires a costly system call to the operating system, which is far more expensive than a simple function call. As the threads run, the OS must perform constant context switches to juggle which thread uses the CPU. This switching process is pure overhead; if threads are frequently forced to wait without being able to perform useful work, performance can easily fall below that of a single-threaded implementation.

Additionally, this implementation uses a single global lock. This mutex forces threads to wait on one another even when modifying different parts of the table, eliminating any potential for parallelism by serializing all write operations. This high contention, combined with the synchronization overhead from the lock calls themselves, makes the program's performance worse than a single-threaded equivalent that has neither thread management nor locking costs.

## Second Implementation
In the `hash_table_v2_add_entry` function, I TODO

### Performance
```shell
TODO how to run and results
```

TODO more results, speedup measurement, and analysis on v2

## Cleaning up
```shell
make clean
```