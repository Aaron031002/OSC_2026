#include "stdlib.h"
#include <stdint.h>

#define HEAP_BYTES (16U * 1024U)
#define ALIGNMENT 16U
#define ALIGN_UP(x) (((x) + ALIGNMENT - 1U) & ~(size_t)(ALIGNMENT - 1U))

struct block {
    size_t size;             /* User payload size, excluding header */
    struct block *next;      /* Following physical block */
    int is_free;
};

#define HEADER_BYTES ALIGN_UP(sizeof(struct block))

static unsigned char heap[HEAP_BYTES] __attribute__((aligned(16)));
static struct block *head;

static void heap_init(void)
{
    if (head) return;
    head = (struct block *)heap;
    head->size = HEAP_BYTES - HEADER_BYTES;
    head->next = NULL;
    head->is_free = 1;
}

void *malloc(size_t size)
{
    if (!size || size > HEAP_BYTES - HEADER_BYTES) return NULL;
    if (size > SIZE_MAX - (ALIGNMENT - 1U)) return NULL;

    size = ALIGN_UP(size);
    heap_init();

    for (struct block *b = head; b; b = b->next) {
        if (!b->is_free || b->size < size) continue;

        if (b->size - size >= HEADER_BYTES + ALIGNMENT) {
            struct block *tail = (struct block *)
                ((unsigned char *)b + HEADER_BYTES + size);
            tail->size = b->size - size - HEADER_BYTES;
            tail->next = b->next;
            tail->is_free = 1;
            b->size = size;
            b->next = tail;
        }

        b->is_free = 0;
        return (unsigned char *)b + HEADER_BYTES;
    }
    return NULL;
}

void free(void *ptr)
{
    if (!ptr || !head) return;

    struct block *found = NULL;
    for (struct block *b = head; b; b = b->next) {
        if ((unsigned char *)b + HEADER_BYTES == (unsigned char *)ptr) {
            found = b;
            break;
        }
    }
    if (!found) return; /* Only pointers returned by malloc are valid. */
    found->is_free = 1;

    /* Merge adjacent free blocks so repeated FDT queries can reuse space. */
    for (struct block *b = head; b && b->next;) {
        if (b->is_free && b->next->is_free) {
            b->size += HEADER_BYTES + b->next->size;
            b->next = b->next->next;
        } else {
            b = b->next;
        }
    }
}
