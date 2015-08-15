#include <stdio.h>
#include <string.h>

#define __USE_LARGEFILE64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "huffman.h"


int main(int argc, char **argv)
{
    huf_reader_t reader;
    huf_writer_t writer;

    huf_error_t (*process)(huf_reader_t, huf_writer_t, uint64_t);

    if (argc < 4) {
        fprintf(stderr, "Usage: [-c] [-x] IFILENAME OFILENAME\n");
        return -1;
    }

    if (!strcmp(argv[1], "-c")) {
        process = huf_encode;
    } else if (!strcmp(argv[1], "-x")) {
        process = huf_decode;
    } else {
        fprintf(stderr, "Usage: [-c] [-x] IFILENAME OFILENAME\n");
        return -1;
    }

    char *reader_name = argv[2];
    char *writer_name = argv[3];

    struct stat64 st;
    stat64(reader_name, &st);

    reader = open(reader_name, O_LARGEFILE | O_RDONLY);
    if (reader < 0) {
        fprintf(stderr, "Open file %s error.\n\n", reader_name);
        fprintf(stderr, "It seems that this file does not exists\n");
        fprintf(stderr, "or you do not have permission to read it.\n");
        return -1;
    }

    writer = open(writer_name, O_LARGEFILE | O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (writer < 0) {
        fprintf(stderr, "Open file %s error.\n\n", writer_name);
        fprintf(stderr, "It seems that this file does not exists\n");
        fprintf(stderr, "or you do not have write permission on this file.\n");
        return -1;
    }

    if (process(reader, writer, st.st_size) != 0) {
        fprintf(stderr, "File processing failed.\n");
        return -1;
    }

    close(reader);
    close(writer);

    return 0;
}
