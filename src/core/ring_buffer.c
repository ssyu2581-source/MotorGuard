#include "mg/ring_buffer.h"
#include <stddef.h>

static uint32_t next_index(const mg_ring_buffer_t *rb, uint32_t index)
{
    uint32_t next = index + 1u;
    return (next == rb->capacity) ? 0u : next;
}

void mg_rb_init(mg_ring_buffer_t *rb, float *storage, uint32_t capacity)
{
    if (rb == NULL) {
        return;
    }

    rb->data = storage;
    rb->capacity = (storage != NULL && capacity >= 2u) ? capacity : 0u;
    rb->head = 0u;
    rb->tail = 0u;
}

uint32_t mg_rb_usable_capacity(const mg_ring_buffer_t *rb)
{
    if (rb == NULL || rb->capacity == 0u) {
        return 0u;
    }
    return rb->capacity - 1u;
}

uint32_t mg_rb_count(const mg_ring_buffer_t *rb)
{
    uint32_t head;
    uint32_t tail;

    if (rb == NULL || rb->capacity == 0u) {
        return 0u;
    }

    head = rb->head;
    tail = rb->tail;
    return (head >= tail) ? (head - tail) : (head + rb->capacity - tail);
}

bool mg_rb_is_empty(const mg_ring_buffer_t *rb)
{
    if (rb == NULL || rb->capacity == 0u) {
        return true;
    }
    return rb->head == rb->tail;
}

bool mg_rb_is_full(const mg_ring_buffer_t *rb)
{
    if (rb == NULL || rb->capacity == 0u) {
        return true;
    }
    return next_index(rb, rb->head) == rb->tail;
}

bool mg_rb_push(mg_ring_buffer_t *rb, float value)
{
    if (rb == NULL || rb->data == NULL || rb->capacity == 0u || mg_rb_is_full(rb)) {
        return false;
    }

    rb->data[rb->head] = value;
    rb->head = next_index(rb, rb->head);
    return true;
}

bool mg_rb_pop(mg_ring_buffer_t *rb, float *value)
{
    if (value == NULL || rb == NULL || rb->data == NULL || mg_rb_is_empty(rb)) {
        return false;
    }

    *value = rb->data[rb->tail];
    rb->tail = next_index(rb, rb->tail);
    return true;
}

bool mg_rb_peek(const mg_ring_buffer_t *rb, float *value)
{
    if (value == NULL || rb == NULL || rb->data == NULL || mg_rb_is_empty(rb)) {
        return false;
    }

    *value = rb->data[rb->tail];
    return true;
}

void mg_rb_clear(mg_ring_buffer_t *rb)
{
    if (rb == NULL) {
        return;
    }
    rb->head = 0u;
    rb->tail = 0u;
}
