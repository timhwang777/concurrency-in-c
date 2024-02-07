# Concurrency in C
![c-badge](https://img.shields.io/badge/Solutions-blue?logo=C
)

## Table of Contents
- [About the Project](#about-the-project)
- [Getting Started](#getting-started)
  - [Project Overview](#project-overview)
  - [Prerequisites](#prerequisites)
- [Environment Setup](#environment-setup)
- [Build and Run with Make](#build-and-run-with-make)
- [Author](#author)

## About the Project

This project is a multithreaded file compression and decompression program written in C. It uses POSIX threads (pthreads) to divide the work of compressing and decompressing a file among multiple threads, each of which handles a portion of the file independently. The compressed output is then written to a new file, and the decompressed output is written back to the original file.

## Getting Started

### Project Overview

The main components of the project are:

- `file_t` structure: Represents a file with its data and size.
- `thread_t` structure: Contains the arguments passed to each thread, including the data to be compressed or decompressed, the size of the data, and the output file name.
- `compress` function: The function that each thread runs to compress a segment of the input data and writes the compressed output to a buffer.
- `decompress` function: The function that each thread runs to decompress a segment of the input data and writes the decompressed output to a buffer.
- `main` function: Reads the input files, creates the threads, and writes the output.

### Prerequisites

To build and run this project, you need:

- A Unix-like operating system
- The GCC compiler
- The pthreads library

## Environment Setup

To compile the program, use the following commands:

```bash
gcc -o pzip pzip.c -lpthread
gcc -o punzip punzip.c -lpthread
```

To run the program, use the following command:
```bash
./pzip file1 [file2 ...]
./punzip file1 [file2 ...]
```
Replace `file1 [file2 ...]` with the names of the files you want to compress.

## Build and Run with Make
If you are using a Linux environment, you can use `make` to build and run the project.

To build the project, use the following command:
```bash
make
```
To compress a file, use the following command:
```bash
make run-pzip
```

To decompress a file, use the following command:
```bash
make run-punzip
```

## Author
Timothy Hwang