#ifndef THREAD_INTERNAL
#define THREAD_INTERNAL

#include "list.h"

typedef struct thread {
    void *stackaddr;
    int stacksize;
    int tid;
    int prio;
    void *(*start_routine)(void *args);
    node_t node;
} thread_t;

dllist_t *thread_list;
void init_threadLib();
#endif // !THREAD_INTERNAL
