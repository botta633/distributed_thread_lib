#include "list.h"
#include <assert.h>

void add_node(dllist_t *list, node_t *node) {
    if (!list || !node)
        return;

    if(!list->head) {
        init_node(node);
        list->head = node;
        return;
    }
    node_t *head = list->head;
    node->next = head;
    head->prev = node;
    list->head = node;

}

static void __delete_node(node_t *node) {
    node_t *prev = node->prev;
    node_t *next = node->next;

    if(next)
        next->prev = prev;
    if(prev)
        prev->next = next;

    free(node);
    // for sanity
    node = 0;
}

void delete_node(dllist_t *list, node_t *node) {
    if(!list || !node)
        return;

    node_t *temp = list->head;

    while(temp) {
        if(temp == node)
            break;
        temp = temp->next;

    }
    if(!temp)
        return;
    if(temp == list->head) {
        list->head = temp->next;
    }

    __delete_node(node);
}


/*
 * Driver program for testing the list
int main() {
    dllist_t *list = malloc(sizeof(dllist_t));
    
    node_t *node_1 = malloc(sizeof(node_t));
    node_t *node_2 = malloc(sizeof(node_t));
    node_t *node_3 = malloc(sizeof(node_t));

    add_node(list, node_1);
    add_node(list, node_2);
    add_node(list, node_3);

    assert(node_3->next == node_2);
    assert(node_2->next == node_1);
    assert(node_1->next == NULL);
    
    assert(node_1->prev == node_2);
    assert(node_2->prev == node_3);
    assert(node_3->prev == NULL);
    assert(list->head == node_3);

    delete_node(list, node_2);
    assert(node_3->next == node_1);
    assert(node_1->prev == node_3);

    delete_node(list, node_3);
    assert(list->head == node_1);
    assert(node_1->next == NULL);
    assert(node_1->prev == NULL);

    delete_node(list, node_1);
    assert(list->head == NULL);

    return 0;

}
*/





