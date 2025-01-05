#include "list.h"
#include "thread_internal.h"

dllist_t *thread_list;


void init_threadLib() {

    thread_list = malloc(sizeof(dllist_t));
    thread_list->head = NULL;
    thread_list->offset = offsetof(thread_t, node);

}

