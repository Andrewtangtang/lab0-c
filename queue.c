#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head == NULL) {
        return NULL;
    }
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head || list_empty(head)) {
        free(head);
        return;
    }

    element_t *entry = NULL, *safe = NULL;
    /* cppcheck-suppress unusedLabel */
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);
    free(head);
}


static element_t *new_element(char *s)
{
    element_t *e = malloc(sizeof(element_t));
    if (!e)
        return NULL;

    e->value = strdup(s);
    if (!e->value) {
        free(e);
        return NULL;
    }
    return e;
}


/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    element_t *element = new_element(s);
    if (!element) {
        return false;
    }

    list_add(&element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    element_t *element = new_element(s);
    if (!element) {
        return false;
    }
    list_add_tail(&element->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (list_empty(head)) {
        return NULL;
    }
    element_t *entry = list_first_entry(head, element_t, list);
    if (sp && entry->value) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del_init(&entry->list);
    return entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (list_empty(head)) {
        return NULL;
    }
    element_t *entry = list_last_entry(head, element_t, list);
    if (sp && entry->value) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del_init(&entry->list);
    return entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;

    struct list_head *current;
    list_for_each (current, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return false;
    }

    struct list_head *slow = head->next;
    struct list_head *fast = head->next->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    list_del_init(slow);
    q_release_element(list_entry(slow, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return false;
    }
    struct list_head **indirect = &head->next;

    while (*indirect != head) {
        const char *cur_val = list_entry(*indirect, element_t, list)->value;
        bool dup = false;

        while ((*indirect)->next != head &&
               strcmp(cur_val,
                      list_entry((*indirect)->next, element_t, list)->value) ==
                   0) {
            dup = true;
            struct list_head *dup_node = (*indirect)->next;
            list_del(dup_node);
            q_release_element(list_entry(dup_node, element_t, list));
        }

        if (dup) {  // the head of duplicate nodes must be deleted
            struct list_head *temp = *indirect;
            list_del_init(*indirect);  // indirect would be automatically
                                       // updated
            q_release_element(list_entry(temp, element_t, list));
        } else {
            indirect = &(*indirect)->next;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    q_reverseK(head, 2);
}

void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *curr = head;
    do {
        struct list_head *tmp = curr->next;
        curr->next = curr->prev;
        curr->prev = tmp;
        curr = tmp;
    } while (curr != head);
}

void reverse_segment(struct list_head *head, struct list_head *tail)
{
    struct list_head *before_head = head->prev;

    struct list_head *curr = head;
    struct list_head *prev = tail;

    while (curr != tail) {
        struct list_head *next = curr->next;
        curr->next = prev;
        curr->prev = next;
        prev = curr;
        curr = next;
    }


    before_head->next = prev;
    prev->prev = before_head;
    head->next = tail;
    tail->prev = head;
}


/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head))
        return;

    struct list_head *curr = head->next;
    while (curr != head) {
        int count = 0;
        struct list_head *tail = curr;
        while (count < k && tail != head) {
            count++;
            tail = tail->next;
        }
        if (count < k)
            break;

        reverse_segment(curr, tail);  // reverse the segment [curr, tail)
        curr = curr->next;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    struct list_head *curr = head->next;
    struct list_head **stack =
        malloc(sizeof(struct list_head *) * q_size(head));
    int top = -1;

    while (curr != head) {
        while (top >= 0 &&
               strcmp(list_entry(stack[top], element_t, list)->value,
                      list_entry(curr, element_t, list)->value) > 0) {
            list_del_init(stack[top]);
            q_release_element(list_entry(stack[top], element_t, list));
            top--;
        }
        stack[++top] = curr;
        curr = curr->next;
    }
    free(stack);
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    struct list_head *curr = head->next;
    struct list_head **stack =
        malloc(sizeof(struct list_head *) * q_size(head));
    int top = -1;

    while (curr != head) {
        while (top >= 0 &&
               strcmp(list_entry(stack[top], element_t, list)->value,
                      list_entry(curr, element_t, list)->value) < 0) {
            list_del_init(stack[top]);
            q_release_element(list_entry(stack[top], element_t, list));
            top--;
        }
        stack[++top] = curr;
        curr = curr->next;
    }
    free(stack);
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
