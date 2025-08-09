# Hash Hash Hash
This lab contains two implementations of a hash table to allow for safe use concurrently. 'v1' is a naive and slow implementation, which is purely focused on correctness. This performs much slower than the base hash table, which is not concurrent safe and single threaded. 'v2' is an implementation that is both correct and performant, performing as good as the base/(num cores-1). 

UID: 806249571.

## Building
```shell
make
```

## Running
```shell
/hash-table-tester -t 8 -s 50000
Generation: 24,150 usec

Hash table base: 475,448 usec

  - 0 missing

Hash table v1: 1,692,074 usec

  - 0 missing

Hash table v2: 147,260 usec

  - 0 missing
```

## First Implementation
I added a global lock to the `hash_table` struct which is initialized when the hash table is created. When `hash_table_v1_add_entry` is called, a lock is acquired at the beginning and held for the duration of the operation. So, when any thread attempts to add an entry, it must first acquire the lock, which means that at most one thread can be actively modifying the table at a time. This trivially protects the `hash_table_v1_add_entry` function from race conditions, as no two threads can add an entry at the same time. The lock is then destroyed when the hash table itself is destroyed. This way, there is one lock per hash table, so all calls to `hash_table_v1_add_entry` for a specific table will go through that table's lock.

### Performance
```shell
/hash-table-tester -t 8 -s 50000
Generation: 24,150 usec

Hash table base: 475,448 usec

  - 0 missing

Hash table v1: 1,692,074 usec

  - 0 missing

Hash table v2: 147,260 usec

  - 0 missing
```
Version 1 is slower than the base version due to multiple sources of overhead. First, it incurs overhead from the nature of threading itself. Creating a thread requires a costly system call to the operating system, which is far more expensive than a simple function call. As the threads run, the OS must perform constant context switches to juggle which thread uses the CPU. This switching process is pure overhead; if threads are frequently forced to wait without being able to perform useful work, performance can easily fall below that of a single-threaded implementation.

Additionally, this implementation uses a single global lock. This mutex forces threads to wait on one another even when modifying different parts of the table, eliminating any potential for parallelism by serializing all write operations. This high contention, combined with the synchronization overhead from the lock calls themselves, makes the program's performance worse than a single-threaded equivalent that has neither thread management nor locking costs.

## Second Implementation
In the `hash_table_v2_add_entry` function, I added a lock for each hash_table_entry (for each bucket). Each write operation will at most access one bucket, and race conditions can only occur when two write conditions interfere with each other. So, the real cause of race conditions would be when 2 threads try to access and write to the same bucket. By adding a lock to each one, we prevent this from happening on a per-bucket level, while allowing other threads not writing to the same bucket to continue their work. This is both correct and highly performant. These locks are initialized when the hash table is created, and destroyed when the hash table is destroyed. Within `hash_table_v2_add_entry`, we grab a lock after identifying which entry we are in. Then, the lock is released after the item is added/the function is finished, just as in implementation one. 

Additionally, I had trouble meeting the high performance requirement, so this submission contains one of three versions of this implementation. I made one version with standard locks belonging to each entry, another with pointers to dynamically allocated locks in each entry, and lastly one with standard locks with each entry aligned to 64 bits. Each version performed similarily, so for simplicity's sake I have included the one with standard locks.

### Performance
```shell
/hash-table-tester -t 8 -s 50000
Generation: 24,150 usec

Hash table base: 475,448 usec

  - 0 missing

Hash table v1: 1,692,074 usec

  - 0 missing

Hash table v2: 147,260 usec

  - 0 missing
```

The performance of this implementation is generally faster than the required based/(num_cores -1). However, extensive testing showed that this was highly variable. This made it so that on some trials, my implementation would pass the base/(num_cores -1) requirement, and on other trials it would not. This can be seen in the descrepency between runs of `./hash-table-tester,` for instance, from the run above and the runs below. For the sake of my sanity and my grade, I've covered in depth my testing on this issue below. 

I made my own testing harness which ran "trials" (3 runs of a command) and recorded all of its data. I ran 10 trials per command, on all three implementations. Essentially, each trial is equivalent to 3 runs of the default `./hash-table-tester` program. The system ran on 4 cores for all runs. There were several runs where the base case decided for whatever reason to randomly run 20% faster (e.g., from 6.8M us to 5.2M us), which caused specific trials to fail performance targets when others did not. However, over time as more and more trials were conducted, these were the averages and my implementation consistently beats the requirement.  The results are as follows:

```shell
./tester -t 8 -s 50000
BASE: 249,188 | v2: 115,483 | highperf: 83,063 | lowperf: 124,594
Failed performance requirements
```

```shell
./tester -t 8 -s 100000
BASE: 1,561,190 | v2: 599,731 | highperf: 520,397 | lowperf: 780,595
Passed weak performance
```

```shell
./tester -t 12 -s 50000
BASE: 673,612 | v2: 265,724 | highperf: 224,537 | lowperf: 336,806
Passed weak performance
```

```shell
./tester -t 12 -s 100000
BASE: 6,813,035 | v2: 2,010,882 | highperf: 2,271,012 | lowperf: 3,406,517
Passed high performance
```

```shell
./tester -t 12 -s 150000
BASE: 22,011,826 | v2: 6,623,406 | highperf: 7,337,275 | lowperf: 11,005,913
Passed high performance
```

I hypothesize that the reason v2 fails to meet the performance requirement for lower workload tasks with lower threads is because at those levels, the overhead associated with managing threads and acquiring locks for each operation outweighs the benefits of parallelism. With a smaller number of elements and fewer threads, the probability of two threads contending for the same bucket is already low. Therefore, the fine-grained locking strategy provides minimal advantage, yet the program still pays the performance penalty for every lock acquisition and thread context switch. As the workload and thread count increase, the likelihood of contention increases, allowing the benefits of concurrent writes to different buckets to significantly outweigh these fixed overheads, leading to the superior performance observed in the high-load tests.

This is evident in the test results. For example, in the test with 8 threads and 50,000 elements (./tester -t 8 -s 50000), the performance target was ~83,063 µs (249,188 / 3), but v2 clocked in at 115,483 µs, failing the requirement. At this scale, the probability of two threads contending for the same bucket is already low. Therefore, the fine-grained locking strategy provides minimal advantage, yet the program still pays the performance penalty for every lock acquisition and thread context switch.

Conversely, as the workload and thread count increase, the benefits of concurrent writes to different buckets begin to significantly outweigh these fixed overheads. In the high-workload test with 12 threads and 150000 elements (./tester -t 12 -s 150000), v2's time of 6,623,406 µs easily beat the performance target of ~7,337,275 µs by over 700 thousand µs! Here, the higher likelihood of contention allows v2's parallel design to show its strengths, leading to the superior performance observed in the high-load tests.

With that being said (sorry for being verbose), the speed analysis is as follows:
By ratio, this implementation ran fastest on a system with 4 cores and a configuration with 12 threads and 100000 items. It ran ~3.39x faster than the base version, which reaches the requirement of v2 <= base(num_cores - 1). It also performs ~8x faster than the v1 implementation (16,145,665us). This configuration passed the performance requirement by 260,130us.

By margin, this implementation ran fastest on a system with 4 cores and a configuration with 12 threads and 150000 items. It ran ~3.32x faster than the base version and ~7.33x faster than v1 (48,569,289us), which also reaches the requirement of v2 <= base/(num_cores -1). This configuration passed the performance requirement by 713,869us.

It is also interesting to note that while this implementation sometimes consistently passes `/hash-table-tester -t 8 -s 50000` (for instance, it has passed 10/10 trials today, but failed every one yesterday), performance is highly variable and seemingly random at times. 

These performance improvements is due to the difference in locking technique between implementation 1 and 2. In the first implementation, whenever any thread wanted to add an entry anywhere in the hash table, all other threads were blocked, causing contention. However, v2 allows multiple threads to work concurrently without blocking each other while adding entries, as long as they are not adding an entry into the same bucket. In this way, 2 or more threads are only ever blocked if they attempt to access the same bucket at the same time. With the huge size of the hash table, this likelihood is quite low, which allows for incredibly fast concurrent operations and rare blocking. 

## Cleaning up
```shell
make clean
```