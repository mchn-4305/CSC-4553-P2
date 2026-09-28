#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

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
            execvp(p->program, p->args);
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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        //invalid input
        fprintf(stderr, "Usage: ./schedule <quantum-ms> <input-file>\n");
        exit(EXIT_FAILURE);
    }

    char *end;
    long quantum = strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0') {
        fprintf(stderr, "Invalid quantum: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    char *filename = argv[2];
    FILE *file = fopen(filename, r);

    if (file == NULL) {
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t capacity = 0;
    while (getline(&line, &capacity, file) != -1) {
        printf("LINE: %s", line);
    }

    free(line);
    fclose(file);
    return EXIT_SUCCESS;

    //After file is processed into linked-list
    //TODO: Get process head
    setup_sigs();
    create_all_processes(head);

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
        group = end;
    }

    
    
    return 0;
}