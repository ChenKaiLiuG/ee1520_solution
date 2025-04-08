# myHeal

## Introduction

`myHeal` is a command-line utility designed to reassemble files that have been previously split into smaller chunks using `myBreak` (or a similar file splitting tool). It requires specifying the destination filename, the filename prefix of the chunks, the expected size of each chunk in kilobytes, and the total number of chunks.

## Compilation

To compile the `myHeal` program, ensure you have a C++ compiler (like g++) installed on your system. Navigate to the directory containing `myHeal.cpp` and execute the following command:

```bash
g++ myHeal.cpp -o myheal
