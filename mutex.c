#include "mutex.h"
#include <err.h>
#include <sys/errno.h>
#include <linux/futex.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdatomic.h>

static int futex(uint32_t *uaddr, int futex_op, uint32_t val,
        const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3)
{
    return syscall(SYS_futex, uaddr, futex_op, val,
            timeout, uaddr2, val3);
}


void mutex_init(mutex_t *m) {
    atomic_store(&m->lock, 0);
}

void thread_mutex_lock(mutex_t *m) {
    int expected = false;
    long s;
    pid_t owner = syscall(SYS_gettid);

    if(owner == m->owner)
        return;

    while(!atomic_compare_exchange_weak(&m->lock, &expected, 1)) {
        expected = false;
        //TODO-> to revisit the pointer comparison
        // check if it should be 0 or 1
        uint32_t *addr = &(*(uint32_t*)(&m->lock));
        s = futex(addr, FUTEX_WAIT, 1, NULL, NULL, 0);
        if (s == -1 && errno != EAGAIN)
            err(EXIT_FAILURE, "futex-FUTEX_WAIT");
    }
    m->owner = owner;
}

void thread_mutex_unlock(mutex_t *m)
{
    if(atomic_load(&m->lock) == 0)
        return;
    atomic_store(&m->lock, 0);
    uint32_t *addr = &(*(uint32_t*)(&m->lock));
    futex(addr, FUTEX_WAKE, 1, NULL, NULL, 0);
}


