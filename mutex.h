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
} mutex_t;

void thread_mutex_lock(mutex_t *);
void thread_mutex_unlock(mutex_t *);
void mutex_init(mutex_t *);
#endif
