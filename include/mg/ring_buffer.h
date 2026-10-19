#ifndef MG_RING_BUFFER_H
#define MG_RING_BUFFER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float *data;
    uint32_t capacity;
    volatile uint32_t head;
    volatile uint32_t tail;
} mg_ring_buffer_t;

void mg_rb_init(mg_ring_buffer_t *rb, float *storage, uint32_t capacity);
uint32_t mg_rb_usable_capacity(const mg_ring_buffer_t *rb);
uint32_t mg_rb_count(const mg_ring_buffer_t *rb);
bool mg_rb_is_empty(const mg_ring_buffer_t *rb);
bool mg_rb_is_full(const mg_ring_buffer_t *rb);
bool mg_rb_push(mg_ring_buffer_t *rb, float value);
bool mg_rb_pop(mg_ring_buffer_t *rb, float *value);
bool mg_rb_peek(const mg_ring_buffer_t *rb, float *value);
void mg_rb_clear(mg_ring_buffer_t *rb);

#ifdef __cplusplus
}
#endif

#endif
