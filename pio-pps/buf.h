#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct cir_buf {
    uint32_t *buffer;
    int max;
    int size;
    int head;
};

struct cir_buf* cir_buf_init(uint32_t *buffer, int size);

void cir_buf_push(struct cir_buf* buf, uint32_t value);

double cir_buf_avg(struct cir_buf* buf);

void cir_buf_print(struct cir_buf* buf);