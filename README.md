# RSFS - Ridiculously Simple File System

**Author:** Aryan Saxena

---

## Overview

RSFS is a ridiculously simple file system implemented in C. It simulates core file system
operations in memory, including file creation, reading, writing, appending, seeking, and
deletion. The full API is defined in `api.h`.

---

## Files Added / Modified

1. **api.c**
   - Implementation of the file system API.
   - Contains all core operations: init, create, open, read, write, append, fseek, close, delete, and stat.

2. **inode.c**
   - Manages inode allocation and deallocation.
   - Tracks the inode bitmap and inode table.

3. **open_file_table.c**
   - Manages the open file table entries.
   - Handles allocation and freeing of file descriptors.

4. **Makefile**
   - Build configuration for compiling the project.

5. **build/** _(folder)_
   - Contains the compiled output.
   - **IMPORTANT:** The executable `app.exe` lives inside this folder.

6. **dev_notes.txt**
   - Personal development notes documenting my thought process while working through
     different parts of the assignment — problem breakdowns, decisions made, and lessons learned.

7. **README.md** _(this file)_
   - Project overview and file descriptions.

8. **transcript.json**
   - contains the transcript of conversation with Claude to help develop, debug and document this project

---

## How to Build

Run the following from the project root:

    make

The compiled binary will be placed in the `build/` folder as `app.exe`.

---

## How to Run

    ./build/app.exe

---

## API Summary

The full API is declared in `api.h`. Key functions include:

    RSFS_init()                        -- Initialize the file system (call this first)
    RSFS_create(char file_name)        -- Create a new file
    RSFS_open(char file_name, int flag)-- Open a file (RSFS_RDONLY or RSFS_RDWR)
    RSFS_append(int fd, void *buf, int size)  -- Append data to a file
    RSFS_fseek(int fd, int offset)     -- Move the current position in an open file
    RSFS_read(int fd, void *buf, int size)    -- Read data from the current position
    RSFS_write(int fd, void *buf, int size)   -- Write data at the current position
    RSFS_close(int fd)                 -- Close an open file
    RSFS_delete(char file_name)        -- Delete a file
    RSFS_stat()                        -- Print file system status and usage

---

## Notes

- File names are single characters (type `char`).
- The file system is entirely in-memory; nothing is persisted to disk.
- Thread safety is handled via pthreads mutexes on shared structures (bitmaps, inodes, open file table).
- Data blocks are allocated with `malloc` on init. Zero-initialize blocks on allocation
  to avoid garbage values from uninitialized memory.
