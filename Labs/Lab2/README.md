## UID: 806249571

## Pipe Up

This program is a recreation of the default pipe operator (|), where users can input multiple programs which will take as input the output of the prior program.

## Building

To build the program, run ``make``. 

To run the program, run:
``./pipe [args]``

## Running

Suppose we run: ``./pipe ls wc cat`` in a directory containing only the contents of this submission after building. 
The expected output would be something like:
 ``6       6      51``


## Cleaning up

To clean up this program's binary files, run ``make clean``