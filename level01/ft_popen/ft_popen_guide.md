# 🧵 ft_popen 

### A clean and student-friendly explanation for understanding UNIX pipes and implementing `popen()` manually



## 📌 Table of Contents

- [🧵 ft\_popen](#-ft_popen)
		- [A clean and student-friendly explanation for understanding UNIX pipes and implementing `popen()` manually](#a-clean-and-student-friendly-explanation-for-understanding-unix-pipes-and-implementing-popen-manually)
	- [📌 Table of Contents](#-table-of-contents)
- [Introduction](#introduction)
- [What Is a Pipe?](#what-is-a-pipe)
- [Understanding File Descriptors](#understanding-file-descriptors)
- [How `pipe(fds)` Works](#how-pipefds-works)
- [How `fork()` Works](#how-fork-works)
- [How Child and Parent Share the Pipe](#how-child-and-parent-share-the-pipe)
- [Using `dup()` vs `dup2()`](#using-dup-vs-dup2)
	- [dup(old\_fd)](#dupold_fd)
	- [dup2(old\_fd, new\_fd)](#dup2old_fd-new_fd)
- [How Redirection Works With `dup2`](#how-redirection-works-with-dup2)
- [Step-by-Step ft\_popen Execution](#step-by-step-ft_popen-execution)
	- [type == 'r': `parent reads`, `child writes`](#type--r-parent-reads-child-writes)
		- [Child does:](#child-does)
		- [Parent does:](#parent-does)
	- [type == 'w': `parent writes`, `child reads`](#type--w-parent-writes-child-reads)
		- [Child:](#child)
		- [Parent:](#parent)
- [EOF Behavior and Why Closing Is Important](#eof-behavior-and-why-closing-is-important)
		- [Parent reads until EOF](#parent-reads-until-eof)
		- [Child reads until EOF](#child-reads-until-eof)
- [Your Final ft\_popen Code](#your-final-ft_popen-code)
- [Visual FD Table Timeline](#visual-fd-table-timeline)
	- [Before fork:](#before-fork)
	- [After fork:](#after-fork)
	- [After child dup2 for `read` (type=='r'):](#after-child-dup2-for-read-typer)
	- [After parent closes unused end:](#after-parent-closes-unused-end)
- [Conclusion](#conclusion)



# Introduction

This document explains **everything** you need to understand to
implement a minimal version of `popen()` using:

-   `pipe()`
-   `fork()`
-   `dup2()`
-   `execvp()`
-   proper closing of file descriptors

This guide makes the concepts visual, intuitive, and exam‑ready.



# What Is a Pipe?

A **pipe** is a unidirectional communication channel implemented by the
kernel.

    write end  ───► [ kernel pipe buffer ] ───► read end

`pipe()` gives you **two file descriptors**:

  Index      Meaning
  ---------- -----------
  fds\[0\]   read end
  fds\[1\]   write end

Any data written into `fds[1]` can be read from `fds[0]`.



# Understanding File Descriptors

Every process has a **file descriptor table**:

    fd 0 → STDIN
    fd 1 → STDOUT
    fd 2 → STDERR
    fd 3 → maybe pipe read end
    fd 4 → maybe pipe write end
    ...

When you fork, this **entire table is copied**, but both tables point to
the **same kernel pipe object**.



# How `pipe(fds)` Works

Call:

``` c
int fds[2];
pipe(fds);
```

You get:

    fds[0] = read end
    fds[1] = write end

Example:

    3 → pipe read end
    4 → pipe write end



# How `fork()` Works

`fork()` duplicates the entire process:

-   memory
-   registers
-   **file descriptor table**

So **parent and child both have fds\[0\] and fds\[1\]** and they both
point to the *same pipe buffer*.



# How Child and Parent Share the Pipe

After fork:

    PARENT FD TABLE           CHILD FD TABLE
    =================         =================
    0 → stdin                 0 → stdin
    1 → stdout                1 → stdout
    2 → stderr                2 → stderr
    3 → pipe read end         3 → pipe read end
    4 → pipe write end        4 → pipe write end

They share the **same kernel pipe**, but **separate FD tables**.

Closing a file descriptor in one process **does NOT** affect the FD
table of the other process.



# Using `dup()` vs `dup2()`

## dup(old_fd)

-   Creates a duplicate of `old_fd` in the **lowest available FD
    number**
-   Does **not** allow choosing the new FD index

Example:

    close(1);  
    dup(4);     // returns 1

## dup2(old_fd, new_fd)

-   Forces duplication into `new_fd`
-   If `new_fd` is open → it is closed first
-   Makes `new_fd` refer to the same underlying kernel object as
    `old_fd`

Example:

    dup2(4, 1); // now stdout is pipe write end



# How Redirection Works With `dup2`

To redirect child's stdout into the pipe:

``` c
dup2(fds[1], STDOUT_FILENO);
```

Result:

    stdout (fd 1) → pipe write end

Then:

``` c
close(fds[1]);
```

This closes only **the original** pipe write descriptor.\
FD 1 stays open and points to the pipe.



# Step-by-Step ft_popen Execution

## type == 'r': `parent reads`, `child writes`

### Child does:

    dup2(fds[1], 1);  // redirect stdout → pipe write
    close(fds[0]);    // child doesn't read
    close(fds[1]);    // child closes original duplicate
    execvp(...);      // program writes to stdout → pipe

### Parent does:

    close(fds[1]);    // parent won't write
    return fds[0];    // parent reads child output



## type == 'w': `parent writes`, `child reads`

### Child:

    dup2(fds[0], 0);  // stdin ← pipe read end
    close(fds[1]);    // child won't write
    close(fds[0]);    // child closes original 
    execvp(...);      // program reads to stdin → pipe

### Parent:

    close(fds[0]);    // parent won't read
    return fds[1];    // parent writes to child stdin



# EOF Behavior and Why Closing Is Important

A pipe reaches **EOF** ONLY when **all write ends are closed**.

Examples:

### Parent reads until EOF

-   Child closes its write end automatically when it exits
-   Parent must close its own write end
-   When last writer closes → parent sees EOF

### Child reads until EOF

-   Parent must close write end so child can detect EOF

Failing to close unused ends leads to **deadlocks**.



# Your Final ft_popen Code

``` c
int ft_popen(const char *file, char *const argv[], char type)
{
    int     fds[2]; // fds[read, write]
    int     pid;

    if (!file || !argv || (type != 'w' && type != 'r'))
        return (-1);
    if ((pid = pipe(fds)) == -1)
        return (-1);
    pid = fork();
    if (pid == -1)
        return (close(fds[0]), close(fds[1]), -1);
    if (pid == 0) //child
    {
        if (type == 'r')
            dup2(fds[1], STDOUT_FILENO);
        else // type == 'w'
            dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
        close(fds[1]);
        execvp(file, argv);
        exit(42); // exit child if exec fails
    }
    // main process
    if (type == 'r')
        return (close(fds[1]), fds[0]);
    else // type == 'w'
        return (close(fds[0]), fds[1]);
}
```



# Visual FD Table Timeline

## Before fork:

    Parent:
    0 stdin
    1 stdout
    2 stderr
    3 pipe read
    4 pipe write

## After fork:

    Parent:                Child:
    3 pipe read            3 pipe read
    4 pipe write           4 pipe write

## After child dup2 for `read` (type=='r'):

    Child:
    1 → pipe write end
    4 closed

## After parent closes unused end:

    Parent:
    3 → pipe read end
    4 closed

Child writes → Parent reads\
EOF when child exits → Parent stops reading



# Conclusion

You now understand:

-   what pipes are
-   how fork duplicates file descriptor tables
-   how dup2 reroutes stdin/stdout
-   why closing unused FDs is essential
-   how popen is implemented internally
-   why your ft_popen code works perfectly

Hope this helps, designed with ☕️ by `fkarika`.