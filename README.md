# FST Wave Demo

A demonstration project for working with FST (Fast Signal Trace) waveform files using the libfst library from GTKWave.

## Overview

This project provides a simple example of how to use the libfst library to write FST waveform files. The FST format is a compact and efficient format for storing digital waveform data, commonly used in digital design and verification.

## Project Structure

The project consists of three main components:

1. **FST Writer Demo**: A basic example demonstrating how to write FST waveform files using the libfst library.
2. **FIRRTL Parser**: A parser implementation for FIRRTL (Flexible Intermediate Representation for RTL) language.
3. **FIRRTL FST Demo** (TODO): A planned component that will demonstrate FIRRTL to FST waveform conversion.

## Prerequisites

- C compiler (gcc)
- Make
- Flex (for lexical analysis)
- Bison (for parsing)
- C++ compiler (g++)

## Building

### FST Writer Demo

To build the FST Writer demo project:

```bash
make
```

This will compile the source files and create the executable.

After building, you can run the demo:

```bash
./build/fstdemo
```

or you can build and run the project directly:

```bash
make run
```

### FIRRTL Parser

The parser component is located in the `parser` directory and includes:

- Lexical analyzer (Flex)
- Parser (Bison)
- AST (Abstract Syntax Tree) implementation
- Multi-threaded parsing support

To build the parser:

```bash
cd parser
make
```

The parser supports parsing FIRRTL files and generates an AST representation of the circuit.

To test the parser with FIRRTL files:

```bash
cd parser
make test
```

This will:
1. Build the parser if not already built
2. Parse all FIRRTL files in the `firrtl_fst_demo/test_run_dir` directory
3. Display the AST structure for each parsed file

The AST output includes:
- Node types
- Node names
- Additional information
- Bit widths
- Sign information
- Line numbers
- Hierarchical structure through indentation

## FIRRTL FST Demo (TODO)

This component is planned to demonstrate the conversion from FIRRTL circuit descriptions to FST waveform files. Features to be implemented include:

- Signal tracing and waveform generation
- Integration with the existing FST writer

## Acknowledgments

- libfst library from [GTKWave](https://github.com/gtkwave/gtkwave) by Tony Bybell 
- firParser from [simulator](https://github.com/jaypiper/simulator) by Jaypiper 