#ifndef THREAD_INTERNAL
#define THREAD_INTERNAL
#include "list.h"

#define MAX_THREAD_NUM 100

typedef void *(*FuncPointer)(void *);
typedef void (*voidFuncPointer)(void *);

typedef struct {
  FuncPointer start_routine;
  void *args;
} thread_args;

typedef struct thread {
  void *stackaddr;
  int stacksize;
  int tid;
  int prio;
  int futex_word;
  thread_args *args;
  node_t node;
} thread_t;

static dllist_t *thread_list;
void init_threadLib();
#endif // !THREAD_INTERNAL
