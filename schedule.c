#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#include "process.h"
#include "parser.h"

//Statuses of running one quantum
#define SLICE_RUNNING 0
#define SLICE_STOPPED 1
#define SLICE_EXITED 2

static volatile pid_t current_pid = 0;
static volatile sig_atomic_t slice_result = SLICE_RUNNING;


//Pause child if quantum runs out
static void on_alarm(int sig) {
    (void)sig;
    if (current_pid > 0) {
        kill(current_pid, SIGSTOP);
    }
}

//Send SIGCHLD to a parent whenever a child changes states
static void on_child(int sig) {
    (void)sig;
    int saved_errno = errno;
    int status;

    if (current_pid > 0) {
        pid_t r = waitpid(current_pid, &status, WNOHANG | WUNTRACED);
        if (r == current_pid) {
            if (WIFSTOPPED(status)) {
                slice_result = SLICE_STOPPED;
            }
            else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                slice_result = SLICE_EXITED;
            }
                
        }
    }
    errno = saved_errno;
}

static sigset_t schedule_sigs;

//Install handlers and blocking signals
static void setup_sigs(void) {
    struct sigaction sa = {0};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sa.sa_handler = on_alarm;
    sigaction(SIGALRM, &sa, NULL);
    sa.sa_handler = on_child;
    sigaction(SIGCHLD, &sa, NULL);

    sigemptyset(&schedule_sigs);
    sigaddset(&schedule_sigs, SIGALRM);
    sigaddset(&schedule_sigs, SIGCHLD);
    sigprocmask(SIG_BLOCK, &schedule_sigs, NULL);
}

//fork every child and waits
static void create_all_processes(Process *head) {
    for (Process *p = head; p; p = p->next) {
        pid_t pid = fork();
        if (pid < 0){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        
        if (pid == 0) {
            sigprocmask(SIG_UNBLOCK, &schedule_sigs, NULL);
            raise(SIGSTOP);
            execvp(p->program, p->argv);
            perror("execvp");
            _exit(EXIT_FAILURE);
        }

        int status;
        waitpid(pid, &status, WUNTRACED);
        p->pid = pid;
        p->stopped = 1;
    }
}


static void set_timer(long ms, long interval_ms) {
    struct itimerval t = {0};
    t.it_value.tv_sec = ms / 1000;
    t.it_value.tv_usec = (ms % 1000) * 1000;
    t.it_interval.tv_sec = interval_ms / 1000;
    t.it_interval.tv_usec = (interval_ms % 1000) * 1000;
    setitimer(ITIMER_REAL, &t, NULL);
}

//Give one process one quantum
static void run_slice(Process *p, long quantum) {
    current_pid = p->pid;
    slice_result = SLICE_RUNNING;
    set_timer(quantum, quantum);
    kill(p->pid, SIGCONT);

    while (slice_result == SLICE_RUNNING) {
        sigprocmask(SIG_UNBLOCK, &schedule_sigs, NULL);
        if (slice_result == SLICE_RUNNING) {
            pause();
        }
        sigprocmask(SIG_BLOCK, &schedule_sigs, NULL);
    }

    set_timer(0, 0);
    current_pid = 0;

    if (slice_result == SLICE_EXITED) {
        p->finished = 1;
    }

}

static void insert_process_sorted (Process **head, Process *new_process) {
    if (*head == NULL ||
        new_process->priority < (*head)->priority) {

        new_process->next = *head;
        *head = new_process;
        return;
    }

    Process *current = *head;

    while (current->next != NULL &&
           current->next->priority <= new_process->priority) {

        current = current->next;
    }

    new_process->next = current->next;
    current->next = new_process;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        //invalid input
        fprintf(stderr, "Usage: ./schedule <quantum-ms> <input-file>\n");
        exit(EXIT_FAILURE);
    }

    char *endptr;
    long quantum = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1] || *endptr != '\0' || quantum <= 0) {
        fprintf(stderr, "Invalid quantum: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    char *filename = argv[2];
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t capacity = 0;
    
    Process *head = NULL;

    while (getline(&line, &capacity, file) != -1) {
        Process *process = parse_process_line(line);

        if (process == NULL) {
            fprintf(stderr, "Could not parse process line\n");
            continue;
        }

        insert_process_sorted(
            &head,
            process
        );
    }

    // Testing loop, delete after
    for (Process *p = head; p != NULL; p = =->next) {
        printf("id=%d priority=%d program =%s\n",
            p->id,
            p->priority,
            p->program
        );
        for (int i = 0; i < p->argc; i++) {
            printf(
                "    argv[%d] = %s\n",
                i,
                p->argv[i]
            );
        }
    }

    free(line);
    fclose(file);
    
    // setup_sigs();
    // create_all_processes(head);

    Process *priority_group = head;
    while (priority_group != NULL) {
        Process *end = priority_group;
        int remaining = 0;
        while (end != NULL && end->priority == priority_group->priority) {
            remaining ++;
            end = end->next;
        }

        Process *p = priority_group;
        while (remaining > 0) {
            if (!p->finished) {
                run_slice(p, quantum);
                if (p->finished) {
                    remaining--;
                }
            }
            p = p->next;
            if (p == end) {
                p = priority_group;
            }
        }
        priority_group = end;
    }

    
    
    return 0;
}