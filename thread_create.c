#include "list.h"
#include "thread_internal.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <linux/sched.h>    /* Definition of struct clone_args */
#include <sched.h>          /* Definition of CLONE_* constants */
#include <sys/syscall.h>    /* Definition of SYS_* constants */
#include <unistd.h>
#include <sys/wait.h>
#define PAGE_SIZE 4096
#define STACK_SIZE (2 * 1024 * 1024)
#define INITIAL_PRIO 5

void init_threadLib() {

    thread_list = malloc(sizeof(dllist_t));
    thread_list->head = NULL;
    thread_list->offset = offsetof(thread_t, node);

}

typedef void *(*FuncPointer)(void *);
int thread_create(thread_t *new_thread, FuncPointer start_routine, void *arg)
{
    void *stackaddr;
    void *stacktop;

    stackaddr = mmap(NULL, STACK_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if(!stackaddr)
        return -1;

    stacktop = stackaddr + STACK_SIZE;
    stacktop = (void *)((long)stacktop & ~0xF);
    long func_location = (long)stacktop - 8;
    new_thread->stackaddr = (void *)func_location;
    *((FuncPointer *)func_location) = *start_routine;
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

    long res = syscall(SYS_clone, flags, (void *)func_location);
    printf("%ld\n", res);
   
  
/*
    long res;
   __asm__ (

        "mov rax, 56            \n\t"  
        "mov rdi, %0            \n\t"  
        "lea rsi, %1            \n\t"
        "mov [rsi], %2"
        "syscall                \n\t"
        "ret \n\t"
        : "=r" (res)        // Output operand: store the return value (rax) in child_pid
        : "r" (flags),      // Input: flags (e.g., CLONE_VM)
          "m" (func_location), // Input: child stack address (must be the top of the stack)
          "r" (start_routine)
        : "%rax", "%rdi", "%rsi", "%rdx"  // Clobbered registers
    );
    */

     return 0;
}


/*
void *test_func(void * args) {
    printf("Came here hopefully by another thread\n");
    exit(0);
}

int main() {
    init_threadLib();
    thread_t *new_thread = malloc(sizeof(thread_t));

    thread_create(new_thread, &test_func, NULL);
    
    return 0;
}
*/



