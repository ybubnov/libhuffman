#include "huffman.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char** argv)
{
    int ifd, ofd;
    struct stat st;
    char ifl_name[256] = "/home/kubrick/test/file.txt";
    char ofl_name[256] = "/home/kubrick/test/out.txt";
    huf_ctx_t hctx;
    
    stat(ifl_name, &st);

    if ((ifd = open(ifl_name, O_LARGEFILE | O_RDONLY)) < 0) {
        return -1;
    }

    if ((ofd = open(ofl_name, O_LARGEFILE | O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU)) < 0) {
        return -1;
    }

    if (huf_init(ifd, ofd, st.st_size, &hctx) != 0) {
        return -1;
    }

    huf_decode(&hctx);
    huf_free(&hctx);

    close(ifd);
    close(ofd);

    return 0;
}

