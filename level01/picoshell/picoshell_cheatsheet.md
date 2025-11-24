# Picoshell Cheat Sheet

## Overview

This guide explains how to implement a minimal UNIX-style shell pipeline
using `fork`, `pipe`, `dup2`, and `execvp`.\
It also covers concepts like process creation, file descriptors,
zombies, and waiting for children.

# 1. Process Creation (fork)

`fork()` duplicates the current process.

-   **Parent:** receives PID of child (\>0)
-   **Child:** receives 0
-   Both execute in parallel after the fork
-   Their memory is separate, but **they share the same kernel file
    descriptors (FDs)** at the moment of the fork

# 2. Pipes

Created with:

``` c
pipe(fds);   // fds[0] = read end, fds[1] = write end
```

Pipes are stored in **kernel memory**, so all processes holding FDs to
the pipe can communicate.

# 3. dup2

Redirects one FD into another:

``` c
dup2(oldfd, newfd);
```

Examples: - `dup2(pipe_read, STDIN_FILENO)` → command reads from pipe
instead of terminal. - `dup2(pipe_write, STDOUT_FILENO)` → command
writes to pipe instead of screen.

`dup2()` automatically closes `newfd` before reuse.

# 4. execvp

Replaces the current process with a new program:

``` c
execvp(cmd, argv);
```

-   On success: **does not return**
-   On failure: returns -1 → child must call `exit(1)`

# 5. Why Close FDs?

Every FD refers to the same kernel pipe object.

-   If ANY write-end of a pipe stays open → **read() never receives
    EOF**.
-   Always close FDs you don't use.

Example: - Child writing → close read-end. - Child reading → close
write-end. - Parent must close both ends appropriately.

# 6. Zombies

When a child exits: - It becomes **zombie** until parent calls `wait()`
or `waitpid()` - Zombie keeps exit code + metadata in kernel - Not
cleaned until parent waits

Always wait for all children to avoid zombie accumulation.

# 7. wait() vs waitpid()

### wait()

-   Waits for **any** child.
-   Blocks until a child finishes.

### waitpid()

-   Can wait for a specific child
-   Can be non-blocking with `WNOHANG`
-   Used for job control in real shells

# 8. picoshell Pipeline Logic

Given cmds:

    cmd1 | cmd2 | cmd3

Your shell must connect:

    cmd1 stdout → pipe0 → cmd2 stdin
    cmd2 stdout → pipe1 → cmd3 stdin

Using this algorithm:

1.  For each command:
    -   If not last command → create a new pipe
    -   Fork
2.  Child:
    -   If not first cmd → dup2(prev_fd → STDIN)
    -   If not last cmd → dup2(fds\[1\] → STDOUT)
    -   Close unused FDs
    -   execvp()
3.  Parent:
    -   Close previous pipe read end
    -   Keep new pipe read end in `prev_fd`
4.  After loop → wait for all children
5.  If any child exited with non-zero → return 1

# 9. picoshell.c Explained

Below is the full working solution:

``` c
int picoshell(char **cmds[])
{
    pid_t pid;
    int fds[2];
    int status;
    int prev_fd = -1;
    int exit_code = 0;
    int i = 0;

    while (cmds[i])
    {
        if (cmds[i + 1] && pipe(fds) == -1)
            return (1);

        pid = fork();
        if (pid == 0) // child
        {
            if (prev_fd != -1)
            {
                if (dup2(prev_fd, STDIN_FILENO) == -1)
                    exit(1);
                close(prev_fd);
            }

            if (cmds[i + 1])
            {
                close(fds[0]);
                if (dup2(fds[1], STDOUT_FILENO) == -1)
                    exit(1);
                close(fds[1]);
            }

            execvp(cmds[i][0], cmds[i]);
            exit(1);
        }

        if (prev_fd != -1)
            close(prev_fd);
        if (cmds[i + 1])
        {
            close(fds[1]);
            prev_fd = fds[0];
        }

        i++;
    }

    while (wait(&status) != -1)
    {
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            exit_code = 1;
    }

    return (exit_code);
}
```

# 10. Key Concepts to Remember

### Fork:

-   Creates parallel child process

### Pipes:

-   Shared kernel buffer
-   fds\[0\] = read, fds\[1\] = write

### dup2:

-   Redirect STDIN/STDOUT

### execvp:

-   Replaces process image

### Zombies:

-   Dead child waiting for parent to collect exit status

### wait loop:

    while (wait(&status) != -1)

Reaps ALL children.

# 11. Visual Pipeline

    cmd1 ----> pipe0 ----> cmd2 ----> pipe1 ----> cmd3

FD flow per iteration:

    prev_fd -> STDIN
    fds[1]  -> STDOUT

Hope this helps, designed with ☕️ by `fkarika`.