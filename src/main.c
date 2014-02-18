#include "huffman.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    int fd;
    struct stat st;
    char filename[256] = "/home/kubrick/test/file.txt";
    huf_ctx_t hctx;
    
    stat(filename, &st);

    if ((fd = open(filename, O_LARGEFILE | O_RDONLY)) == -1) {
        return -1;
    }

    if (huf_init(fd, st.st_size, &hctx) == -1) {
        return -1;
    }

    huf_decode(hctx);

    printf("here\n");
    huf_free(&hctx);

    return 0;
}
