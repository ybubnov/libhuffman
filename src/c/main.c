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
#include "huffman/sys.h"


int main(int argc, char **argv)
{
    huf_reader_t reader;
    huf_writer_t writer;

    huf_error_t err;

    /*huf_error_t (*process)(const *encoder_confi);*/

    if (argc < 4) {
        fprintf(stderr, "Usage: [-c] [-x] IFILENAME OFILENAME\n");
        return -1;
    }

    if (strcmp(argv[1], "-c") && strcmp(argv[1], "-x")) {
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

    __debug__("Opened a reader\n");

    writer = open(writer_name, O_LARGEFILE | O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (writer < 0) {
        fprintf(stderr, "Open file %s error.\n\n", writer_name);
        fprintf(stderr, "It seems that this file does not exists\n");
        fprintf(stderr, "or you do not have write permission on this file.\n");
        return -1;
    }

    __debug__("Opened a writer\n");

    huf_config_t config = {
        .reader = reader,
        .writer = writer,
        .length = st.st_size,
        .chunk_size = __512KIB_BUFFER,
        .reader_buffer_size = __1MIB_BUFFER,
        .writer_buffer_size = __1MIB_BUFFER,
    };

    if (!strcmp(argv[1], "-c")) {
        err = huf_encode(&config);
    } else {
        err = huf_decode(&config);
    }

    if (err != HUF_ERROR_SUCCESS) {
        fprintf(stderr, "%s\n", huf_err_string(err));
        return -1;
    }

    close(reader);
    close(writer);

    return 0;
}
