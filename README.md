﻿# SeaScript

SeaScript is a simple, typeless programming language made in C.

Seascript is designed to be fast and reliable while maintaining the capabilities of a high-level language.

# Example Program

example.ssc:

    function countToTen() {
        global i = 0;
        
        while i < 10 {
            i = i + 1;
        }
    }

    countToTen();
    print("Successfully counted to 10!");

To run this code:

    ./SeaScript.exe example.ssc

You can also generate a bytecode file like this:

    ./SeaScript.exe example.ssc --preserve

This will create a file `example.ssb` storing the bytecode which can be executed directly:

    ./SeaScript.exe example.ssb
   Which is faster as it doesn't have to recompile the source code each time you wish to run it.

# Flags

- `--version` - Shows the current SeaScript version
- `--visualize-tokens` - Visualizes the token stream generated by SeaScript
- `--parser-print` - Pretty prints the output generated by the parser
- `--view` - Used to view the instructions from a bytecode file
- `--preserve` - Preserves the bytecode generated by SeaScript onto a file which can be executed
- `--benchmark` - Benchmark Seascript code

# Build

To build this project make sure you have GCC installed, afterwards just run:
```
make
```
