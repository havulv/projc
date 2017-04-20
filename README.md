
# PROJC

A C-project directory creator, complete with Makefile generation.

## Why?

I found it really dull to continuously create new Makefiles and directories when 90% of the projects I was doing were one-offs that had a set structure.

## What does it create?

If you will refer to the ASCII tree below, the utility creates a simple directory structure of `include`, `lib`, `src`, and `test`. Within these are some sample header and C files with common structural elements in them. The Makefiles are kind of overbearing to be honest, and will be pared down in later editions.

```
    Project
       |
       |____ include
       |
       |____ src
       |      |
       |      |_____Project_app.c
       |
       |____ lib
       |      |
       |      |_____Project.c
       |      |
       |      |_____Project.h
       |
       |____ test
              |
              |_____Project_test.c
```
