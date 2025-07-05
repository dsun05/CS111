# A Kernel Seedling
This is a kernel module for CS111 Lab 0 that creates a virtual file called count in the /proc filesystem. When users read from /proc/count, it should return the current number of running processes in the system.
Written by David Sun. 806249571

## Building
```shell
make
```

## Running
```shell
sudo insmod proc_count.ko
cat /proc/count
```
TODO: results?

## Cleaning Up
```shell
sudo rmmod proc_count
make clean
```

## Testing
```python
python -m unittest
```
TODO: results?

Report which kernel release version you tested your module on
(hint: use `uname`, check for options with `man uname`).
It should match release numbers as seen on https://www.kernel.org/.

```shell
uname -r -s -v
```
The kernel version of the provided VM OS is Linux 5.14.8-arch1-1