# Programming Assignment 2

# Scheduling

Scheduling is one of the fundamental components of operating systems. 
**Priority Scheduling** associates a priority with each process, and the CPU is allocated to the process with the highest priority first.
For processes with similar priority, another scheduling algorithm can be employed. 
**Round Robin** is a *preemptive* first-come first-served scheduling algorithm. It goes around the ready queue, allocating the CPU to each process for a time interval of up to 1 time quantum, a defined short unit of time.
In this assignment, you will implement your own user-level scheduler that uses **Priority Scheduling with Round-Robin**.


# 1. Program
Your `schedule` program should fork a series of child processes, a child process for each process defined in an input file, adding them into a priority queue. 
A process with a higher priority (lower number) should be running to completion.
Processes with the same priority should circulate, and run in time quantum intervals  using Round Robin. When a process concludes, it should be taken out of circulation.

## 1.1 Input

Your scheduler program should expect two parameters:
- A time quantum in milliseconds
- A tab-separated file where each line in the file represents a process defined as follows:
    - a process identifier: any unique natural number.
    - a process priority: natural number from 0 to 127, with lower values indicating higher priority.
    - a process binary file
    - and optional parameters for the binary file. 

    Your file can have any number of processes with any arbitrary number of parameters.
    An example for the input file:

    **`processes.tsv`**
    ```bash
    8   6   demo    5
    9   2   demo    7
    10  2   test    "hello"     4
    11  2   test    "welcome"   7    
    ```
    
    `processes.tsv` file defines four processes with ids 8, 9, 10 and 11 respectively. 
    Process id 8, for example, has a priority value equals to 6, and is running a demo executable that accepts one integer value.
    
## 1.2 Running your scheduler

The command to run your scheduler:

```bash
./schedule time-quantum input-file 
```

For example, for time quantum equal 500 milliseconds, and for the above file, your command should be:

```bash
./schedule 500 processes.tsv
```

## 1.3. Example

**demo.c**
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int n = atoi(argv[1]);

    for (int i = 0; i < n; i++) {
        printf("%d\n", n);
        sleep(1); // wait 1 second
    }

    return 0;
}
```

**test.c**
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    char *text = argv[1];
    int n = atoi(argv[2]);

    for (int i = 0; i < n; i++) {
        printf("%s\n", text);
        sleep(1); // wait 1 second
    }

    return 0;
}
```

**processes.tsv**
```bash
8   6   demo    5
9   2   demo    7
10  2   test    "hello"     4
11  2   test    "welcome"   5
12  0   demo    3  
```

Running the scheduler with quantum of 2 seconds (2000 milliseconds):

```bash
./schedule 2000 processes.tsv
```

The scheduler should run process 12 first, then run Round Robin over processes 9, 10, and 11, and lastly run process 8. One possible output:

```bash
3
3
3
7
7
hello
hello
welcome
welcome
7
7
hello
hello
welcome
welcome
7
7
welcome
7
5
5
5
5
5
```
# 2. Submission
You should submit your header, source files along with a makefile. 

## 2.1. Requirements

Your program must:

1. be named schedule (binary executable)
2. compile and run on the department general use servers.  
3. use fork() to run all the processes.
4. allow each process to run until either the quantum expires or it terminates or it suspends itself. 
5. maintain the original round-robin scheduling order as processes terminate. 
6. only give quanta to live processes. If a process drops out, it should not be scheduled any more. Respond immediately to process terminations or self-suspensions. A process should not be forced to take its full quantum, and the next process should not have to wait for the quantum to expire. 
7. give each process the opportunity to have a full quantum, regardless of how the previous process gave up its quantum. 
8. continue scheduling processes until there are no more processes left. 
9. dynamically allocate and deallocate resources for each process. 
10. use wait() or (waitpid()) for each child at an appropriate time; you don’t want to leave the system swarming with zombies.  
11. not busy-wait. It can’t just do an infinite poll like a “for(;;);” loop polling for data or signals. If it has nothing to do, it should not be executing anything. You may use: 
    - interval timer (setitimer() with ITIMER_REAL)  to keep track of when quanta have expired. 
    - pause(2).

# 2.2. Hints
1. A process can be stopped by sending it SIGSTOP signal. A stopped process can be restarted with a SIGCONT signal. 
2. Remember that catching a signal interrupts “long” system calls like wait() and pause() and causes them to return. 
3. Develop incrementally. Make sure you can launch and reap all of your children properly before attempting to schedule them. 
5. Think about the problem of initially creating and stopping each child. Right after you run fork() a new process is created and starts executing immediately. But you must not allow it to run until it can be scheduled. 
5. Create test executables to test with the scheduler. 

# 3. Relevant System Calls
| System Call(s) | Description |
|---|---|
| `pid_t fork(2)` | Creates a new process that is an exact image of the current one. |
| `int execl(3)`<br>`int execlp(3)`<br>`int execle(3)`<br>`int execv(3)`<br>`int execve(2)`<br>`int execvp(3)` | Known collectively as "the execs," this family of functions overwrites the current process with a new program. |
| `int kill(2)` | Sends a signal to a process. |
| `void pause(2)` | Suspends execution until a signal is received. |
| `int raise(3)` | Sends a signal to the current process. It is equivalent to `kill(getpid(), sig);` |
| `getitimer(2)`<br>`setitimer(2)` | For getting and setting the system interval timer. |
| `sigaction(2)` | The POSIX reliable interface for signal handling. |
| `sigprocmask(2)` | For manipulating the blocking or unblocking of particular signals. |
| `sigemptyset(3)`<br>`sigfillset(3)`<br>`sigaddset(3)`<br>`sigdelset(3)`<br>`sigismember(3)` | For manipulating signal sets used by `sigaction(2)` and `sigprocmask(2)` |
| `strtol(3)` | Convert a string to an integer. |
| `pid_t wait(2)`<br>`pid_t waitpid(2)` | Waits for a child process to change status (terminate or stop). Usually `wait()` and `waitpid()` block and return only after a child process has been terminated. if you run waitpid() with the WUNTRACED flag, it will also return when a process has been stopped. You should also recall that whenever a process stops or terminates its parent receives a SIGCHLD signal. |


 

> **Source & Acknowledgment:** This assignment is reproduced from Professor Foaad Khosmood