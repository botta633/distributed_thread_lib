#include "cond.h"
#include <linux/futex.h>
#include <unistd.h>

void thread_cond_init(thread_cond_t *cond) { cond->futex_word = 0; }


//TODO-> test this part and check for race conditions
void thread_cond_wait(thread_cond_t *cond, thread_mutex_t *mutex) {
  thread_mutex_unlock(mutex);
  while (cond->futex_word == 0) {
    // Wait for the futex_word to change
    syscall(SYS_futex, cond->futex_word, FUTEX_WAIT, 0, NULL, NULL, 0);
  }
}
void thread_cond_signal(thread_cond_t *cond) {
  cond->futex_word = 1; // Mark thread as complete
  syscall(SYS_futex, cond->futex_word, FUTEX_WAKE, 1, NULL, NULL, 0);
}
