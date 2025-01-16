#ifndef mutex_h
#define mutex_h

#include <stdatomic.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include "thread_internal.h"

typedef struct{
    _Atomic int lock;
    int owner;
} thread_mutex_t;

void thread_mutex_lock(thread_mutex_t *);
void thread_mutex_unlock(thread_mutex_t *);
void mutex_init(thread_mutex_t *);
#endif
