#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
//strechy buffer
typedef struct bufHdr {
    size_t len;
    size_t cap;
    char buf[0];
} bufHdr; 

#define buf__hdr(b) ((bufHdr *)((char *)(b) - offsetof(bufHdr, buf))) // return buffer header
#define buf__fits(b, n) (buf_len(b) + (n) <= buf_cap(b)) // check if buffer fits
#define buf__fit(b, n) (buf__fits(b, n) ? 0 : ((b) = buf__grow((b),buf_len(b)+n,sizeof((*b))))) // grow buffer if needed

#define buf_len(b) ((b)? buf__hdr(b)->len:0) // return buffer length
#define buf_cap(b) ((b)? buf__hdr(b)->cap:0) // return buffer capacity
#define buf_push(b, x) (buf__fit((b), 1), (b)[buf__hdr(b)->len++] = (x)) // push element to buffer

#define buf_free(b) ((b) ? free(buf__hdr(b)), (b) = NULL : 0) // free buffer

void *buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    size_t new_cap = MAX(1 + 2 * buf_cap(buf), new_len);
    size_t new_size = offsetof(bufHdr, buf) + new_cap * elem_size;
    bufHdr *new_hdr;
    if (buf) {
        new_hdr = realloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = malloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}



int main(int argc,char **argv) {
    int *buf=NULL;
    buf_push(buf, 10);
    buf_push(buf, 20);
    buf_push(buf, 30);
    buf_push(buf, 40);
    buf_push(buf, 50);
    for(int i=0;i<buf_len(buf);i++) {
        printf("%d\n", buf[i]);
    }

    buf_free(buf);
    return 0;
}
