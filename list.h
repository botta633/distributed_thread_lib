#ifndef list_h
#define list_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Node {
    struct Node *next;
    struct Node *prev;
} node_t;


typedef struct DLList {
    struct Node *head;
    unsigned int offset;
} dllist_t;


void add_node(dllist_t *list, node_t *node);

void delete_node(dllist_t *list, node_t *node);



#define DLLIST_ITERATE_BEGIN(list_ptr, struct_type, ptr) { \
    node_t *_node = NULL; \
    for (_node = list_ptr->head; _node; _node = _node->next) \
        ptr = (struct_type *)((void *)_node - list_ptr->offset);
#define DLLIST_ITERATE_END }



#define init_node(node)\
    node->prev = node->next = NULL;


// c standard 6.6 paragraph 9
#define offsetof(struct_name, field_name) \
   ((unsigned int)&((struct_name *)NULL)->field_name);



#endif
