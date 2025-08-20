# Hey! I'm Filing Here

In this lab, I successfully implemented a 1 MiB ext2 file system with 2 directories, 1 regular file, and 1 symbolic link.

UID: 806249571

## Building
To build:
```
make
```

## Running
To run:
```
make                                    # compile the executable
./ext2-create                           # run the executable to create cs111 -base.img
mkdir mnt                               # create a directory to mnt your filesystem to
sudo mount -o loop cs111-base.img mnt  # mount your filesystem , loop lets you use a file
```

Sample output:
```
cs111@cs111 Labs/lab5 (master !*%) » ls -ain mnt/

total 7

     2 drwxr-xr-x 3    0    0 1024 Aug 20 15:40 .

942179 drwxr-xr-x 3 1000 1000 4096 Aug 20 15:40 ..

    13 lrw-r--r-- 1 1000 1000   11 Aug 20 15:40 hello -> hello-world

    12 -rw-r--r-- 1 1000 1000   12 Aug 20 15:40 hello-world

    11 drwxr-xr-x 2    0    0 1024 Aug 20 15:40 lost+found

```


## Cleaning up
To clean:
```
sudo umount mnt                         # unmount the filesystem when you're done
rmdir mnt                               # delete the directory used for mounting when you're done
make clean                              # clean executable
```
