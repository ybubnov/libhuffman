#include "huffman.h"
#include "error.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char** argv)
{
    int ifd, ofd;
    huf_ctx_t hctx;
    struct stat64 st;

    char *ifl_name, *ofl_name;
    int (*process)(huf_ctx_t*);

    if (argc < 4) {
        ERROR("Usage: [-c] [-x] ifilename ofilename\n");
        return -1;
    }

    if (!strcmp(argv[1], "-c")) {
        process = huf_encode;
    } else if (!strcmp(argv[1], "-x")) {
        process = huf_decode;
    } else {
        ERROR("Usage: [-c] [-x] ifilename ofilename\n");
        return -1;
    }

    ifl_name = argv[2];
    ofl_name = argv[3];
    
    stat64(ifl_name, &st);

    if ((ifd = open(ifl_name, O_LARGEFILE | O_RDONLY)) < 0) {
        ERROR("Open file %s error.\n\n", ifl_name);
        ERROR("It seems that this file does not exists\n");
        ERROR("or you do not have permission to read it.\n");
        return -1;
    }

    if ((ofd = open(ofl_name, O_LARGEFILE | O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU)) < 0) {
        ERROR("Open file %s error.\n\n", ofl_name);
        ERROR("It seems that this file does not exists\n");
        ERROR("or you do not have write permission on this file.\n");
        return -1;
    }

    if (huf_init(ifd, ofd, st.st_size, &hctx) != 0) {
        return -1;
    }

    if (process(&hctx) != 0) {
        ERROR("File decoding failed.\n");
        return -1;
    }

    huf_free(&hctx);

    close(ifd);
    close(ofd);

    return 0;
}


