# Fytecode
A simple 16-bit assembler + virtual-machine interpreter for a custom bytecode.

Basically, this project provides an assembler for a custom assembly language
that yields custom binary output.

The binary output can be run with the provided virtual-machine.

# General information
The assembly syntax is very similar to the x86 (and specifically 8086) intel syntax,
but has a few deviations such as not using commas (because why not).

The binary output currently isn't that optimized for speed or size, but is a bit more
readable than 8086 assembler binary output.

Also, as opposed to 8086, the virtual machine does not use segments, but just plain
16-bit addresses.

# Why
First of all for fun, but also for learning about assemblers, bytecodes, executables etc...

This isn't really meant to be used by other people and is pretty much just for learning.

# Build & usage
The program was tested only on my Ubuntu 20.04 machine, but would probably
work on other *nix machines.

To build on *nix machines, run `make all`.

The executable will be set in the `build/` directory.

To assemble a file, run `build/fy -c <path-to-file> <path-to-new-executable>`.

To run a generated executable, run `build/fy -r <path-to-executable>`.

# Name
When thinking about a name for the project, I wanted to incorporate the word "bytecode"
with something else. That made me think of words that rhyme with "byte" and I immediately
thought of "fight", then I just glued the words together without any meaning.
