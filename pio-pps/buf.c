#include "buf.h"

struct cir_buf* cir_buf_init(uint32_t *buffer, int size) {
    struct cir_buf *cbuf = malloc(sizeof (struct cir_buf));
    for(int i = 0;i < size;i++) {
        buffer[i] = 0;
    }
    cbuf->buffer = buffer;
    cbuf->size = 0;
    cbuf->head = 0;
    cbuf->max = size;
    return cbuf;
}

void cir_buf_print(struct cir_buf* buf) {
    printf("Size = %d, Head = %d, Max = %d\n", buf->size, buf->head, buf->size);
    for(int i = 0;i < buf->size;i++) {
        printf("%d ", buf->buffer[i]);
    }
    printf("\n");
}

void cir_buf_push(struct cir_buf* buf, uint32_t val) {
    //printf("Adding %d at position %d\n", val, buf->head);
    buf->buffer[buf->head] = val;
    if(buf->size < buf->max) {
        buf->size = buf->size + 1;
    }
    buf->head = (buf->head + 1) % buf->max;
}

double cir_buf_avg(struct cir_buf* buf) {
    uint64_t total = 0;
    for(int i = 0;i < buf->size;i++) {
        total += buf->buffer[i];
    }
    return total / (double)buf->size;
}