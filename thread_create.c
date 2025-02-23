#include "list.h"
#include "mutex.h"
#include "thread_internal.h"
#include "capture_context.h"
#include <err.h>
#include <linux/futex.h>
#include <linux/sched.h> /* Definition of struct clone_args */
#include <pthread.h>
#include <sched.h> /* Definition of CLONE_* constants */
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h> /* Definition of SYS_* constants */
#include <sys/ucontext.h>
#include <ucontext.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/taskstats.h>

#define PAGE_SIZE 4096
#define STACK_SIZE (2 * 1024 * 1024)
#undef REG_R8
#undef REG_R9

void init_threadLib()
{
  thread_list = malloc(sizeof(dllist_t));
  thread_list->head = NULL;
  thread_list->offset = offsetof(thread_t, node);
}

int thread_join(thread_t *thread)
{
  while (thread->futex_word == 0)
  {
    // Wait for the futex_word to change
    syscall(SYS_futex, &thread->futex_word, FUTEX_WAIT, 0, NULL, NULL, 0);
  }

  // Clean up the thread's stack
  munmap(thread->stackaddr, STACK_SIZE);
  return 0;
}
void thread_exit(thread_t *thread)
{
  thread->futex_word = 1; // Mark thread as complete
  syscall(SYS_futex, &thread->futex_word, FUTEX_WAKE, 1, NULL, NULL, 0);
  syscall(SYS_exit);
}
static void thread_start(void *arg)
{
  // Extract thread function and argument from the passed struct
  // Unblock signals in the child thread
  sigset_t mask;
  sigemptyset(&mask);
  sigprocmask(SIG_SETMASK, &mask, NULL);

  thread_t *thread = *(thread_t **)((uintptr_t)__builtin_frame_address(0) + 8);
  thread_args *args = thread->args;
  args->start_routine(args->args); // Execute the start routine

  capture_memory_pages(thread->tid);
  free(args);
  thread_exit(thread);
}

int thread_create(thread_t *new_thread, FuncPointer start_routine, void *arg)
{
  void *stackaddr;
  void *stacktop;
  sigset_t new_mask, old_mask;
  ucontext_t thread_context;
  thread_args *args = malloc(sizeof(thread_args));

  stackaddr = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (!stackaddr)
    return -1;

  args->start_routine = start_routine;
  args->args = arg;

  // TODO ->introduce TLS
  // current behavior passing a pointer to thread_struct on stack

  stacktop = stackaddr + STACK_SIZE;
  long func_location = (long)stacktop - 16;
  long args_location = func_location + 8;
  *((voidFuncPointer *)(func_location)) = thread_start;
  *((thread_t **)(args_location)) = new_thread;

  new_thread->stackaddr = (void *)func_location;
  new_thread->stacksize = STACK_SIZE;
  new_thread->args = args;
  new_thread->futex_word = 0;
  add_node(thread_list, &(new_thread->node));

  int clone_flags =
      (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SYSVSEM | CLONE_SIGHAND |
       CLONE_THREAD | CLONE_PARENT_SETTID | CLONE_CHILD_SETTID);

  // Block all signals before thread creation
  sigfillset(&new_mask);                        // Block all signals
  sigprocmask(SIG_BLOCK, &new_mask, &old_mask); // Save old mask

  printf("Came here 2\n");
  long pid = syscall(SYS_clone, clone_flags, (void *)func_location);
  new_thread->tid = pid;

  if (pid == -1)
  {
    perror("clone failed");
    munmap(stackaddr, STACK_SIZE);
    return -1;
  }

  //  Restore the original signal mask in the parent process
  sigprocmask(SIG_SETMASK, &old_mask, NULL);
  return 0;
}

int y = 3;
int x = 0;
thread_mutex_t mutex;
void *test_func2(void *args)
{
  printf("Came here\n");
  return 0;
}

int main()
{
  init_threadLib();
  // thread_t new_thread[10];
  // create a thread and call capture_context
  thread_t thread;
  thread_create(&thread, &test_func2, NULL);
  thread_join(&thread);

  return 0;
}
