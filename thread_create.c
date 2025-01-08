#include "list.h"
#include "thread_internal.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <linux/sched.h>    /* Definition of struct clone_args */
#include <sched.h>          /* Definition of CLONE_* constants */
#include <sys/syscall.h>    /* Definition of SYS_* constants */
#include <unistd.h>
#include <sys/wait.h>
#include <err.h>
#define PAGE_SIZE 4096
#define STACK_SIZE (2 * 1024 * 1024)
#define INITIAL_PRIO 5

typedef void *(*FuncPointer)(void *);
typedef void (*voidFuncPointer)(void *);

typedef struct {
    FuncPointer start_routine;
    void *args;
} thread_args;

void init_threadLib() {

    thread_list = malloc(sizeof(dllist_t));
    thread_list->head = NULL;
    thread_list->offset = offsetof(thread_t, node);
}

static void thread_start(void *arg)  {
    // Extract thread function and argument from the passed struct
    // Unblock signals in the child thread
    sigset_t mask;
    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    thread_args *args = *(thread_args **)((uintptr_t)__builtin_frame_address(0) + 8);
    args->start_routine(args->args);  // Execute the start routine

    free(args);
    _exit(0);
}

int thread_create(thread_t *new_thread, FuncPointer start_routine, void *arg)
{
    void *stackaddr;
    void *stacktop;
    sigset_t new_mask, old_mask;
    thread_args *args = malloc(sizeof(thread_args));
    printf("address of real args is %p\n", args);

    stackaddr = mmap(NULL, STACK_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if(!stackaddr)
        return -1;

    args->start_routine = start_routine;
    args->args = arg;

    stacktop = stackaddr + STACK_SIZE;
    long func_location = (long)stacktop - 16;
    long args_location = func_location + 8;
    *((voidFuncPointer *)(func_location)) = thread_start;
    *((thread_args **)(args_location)) = args;

    new_thread->stackaddr = (void *)func_location;
    new_thread->stacksize = STACK_SIZE;
    new_thread->start_routine = start_routine;
    add_node(thread_list, &(new_thread->node));

    long flags = 0;
    flags |= CLONE_VM;
    flags |= CLONE_FS;
    flags |= CLONE_FILES;
    flags |= CLONE_SIGHAND;
    flags |= CLONE_THREAD;
    flags |= CLONE_PARENT;
    flags |= CLONE_IO;

    // Block all signals before thread creation
    sigfillset(&new_mask);  // Block all signals
    sigprocmask(SIG_BLOCK, &new_mask, &old_mask);  // Save old mask

    long pid = syscall(SYS_clone, flags, (void *)func_location);
    if (pid == -1) {
        perror("clone failed");
        munmap(stackaddr, STACK_SIZE);
        return -1;
    }

    sleep(3);
    if (waitpid(pid, NULL, 0) == -1) {
        perror("waitpid failed");
        return -1;
    }
    // Restore the original signal mask in the parent process
    sigprocmask(SIG_SETMASK, &old_mask, NULL);
    printf("child has terminated\n");

    return 0;
}

/*
void *test_func(void *args) {
    printf("This is %s\n",(char*)args);
    return 0;
}

int main() {
    init_threadLib();
    thread_t *new_thread = malloc(sizeof(thread_t));

    thread_create(new_thread, &test_func, "Hello from thread");

    return 0;
}

*/

