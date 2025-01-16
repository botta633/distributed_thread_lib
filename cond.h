#ifndef COND_H
#define COND_H
#include "mutex.h"
typedef struct {
    thread_mutex_t *cond_mutex;
    int futex_word;
} thread_cond_t;

void thread_cond_init(thread_cond_t* );
void thread_cond_wait(thread_cond_t*, thread_mutex_t *);
void thread_cond_signal(thread_cond_t*);




#endif // !COND_H
