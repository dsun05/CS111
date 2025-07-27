# You Spin Me Round Robin

This is an implementation of a round robin scheduler in c.

## Building

```shell
make
```

## Running

To run the program
```shell
./rr [input file] [quantum slice]
```

Where the input file is formatted like:
```shell
numprocesses
pid, arrival_time, burst_time
```

Example ``processes.txt``:
```shell
4
1, 0, 7
2, 2, 4
3, 4, 1
4, 5, 4
```

Expected output
```shell
./rr processes .txt 3
Average waiting time: 7.00
Average response time: 2.75
```

## Cleaning up

```shell
make clean
```
