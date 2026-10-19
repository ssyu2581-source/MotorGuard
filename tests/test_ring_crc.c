#include "test_util.h"

#include "mg/crc16.h"
#include "mg/ring_buffer.h"

void test_ring_buffer(void)
{
    float storage[4];
    mg_ring_buffer_t rb;
    float value;

    puts("test_ring_buffer");
    mg_rb_init(&rb, storage, 4u);
    MG_CHECK(mg_rb_usable_capacity(&rb) == 3u, "usable capacity");
    MG_CHECK(mg_rb_is_empty(&rb), "new buffer empty");

    MG_CHECK(mg_rb_push(&rb, 1.0f), "push 1");
    MG_CHECK(mg_rb_push(&rb, 2.0f), "push 2");
    MG_CHECK(mg_rb_push(&rb, 3.0f), "push 3");
    MG_CHECK(mg_rb_is_full(&rb), "buffer full");
    MG_CHECK(!mg_rb_push(&rb, 4.0f), "reject push when full");
    MG_CHECK(mg_rb_count(&rb) == 3u, "count after pushes");

    MG_CHECK(mg_rb_pop(&rb, &value) && value == 1.0f, "pop 1");
    MG_CHECK(mg_rb_pop(&rb, &value) && value == 2.0f, "pop 2");
    MG_CHECK(mg_rb_push(&rb, 4.0f), "push 4 after wrap");
    MG_CHECK(mg_rb_pop(&rb, &value) && value == 3.0f, "pop 3");
    MG_CHECK(mg_rb_pop(&rb, &value) && value == 4.0f, "pop 4");
    MG_CHECK(mg_rb_is_empty(&rb), "buffer empty after pops");
    MG_CHECK(!mg_rb_pop(&rb, &value), "reject pop when empty");

    mg_rb_push(&rb, 9.0f);
    mg_rb_clear(&rb);
    MG_CHECK(mg_rb_is_empty(&rb), "clear empties buffer");
}

void test_crc16(void)
{
    static const uint8_t text[] = "123456789";
    uint16_t crc;
    uint16_t split_crc;

    puts("test_crc16");
    crc = mg_crc16_modbus(text, sizeof(text) - 1u);
    MG_CHECK(crc == 0x4B37u, "Modbus CRC of 123456789");
    split_crc = mg_crc16_modbus_update(0xFFFFu, text, 4u);
    split_crc = mg_crc16_modbus_update(split_crc, text + 4u, sizeof(text) - 5u);
    MG_CHECK(split_crc == crc, "split CRC update");
}