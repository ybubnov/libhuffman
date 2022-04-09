#ifndef INCLUDE_huffman_decoder_h__
#define INCLUDE_huffman_decoder_h__

#include "huffman/config.h"
#include "huffman/common.h"
#include "huffman/errors.h"

#define CFFI_huffman_decoder_h__

// A Huffman decoding context.
typedef struct __huf_decoder huf_decoder_t;


// Initialize a new instance of the Huffman-decoder.
huf_error_t
huf_decoder_init(huf_decoder_t **self, const huf_config_t *config);


// Release memory occupied by the Huffman-decoder.
huf_error_t
huf_decoder_free(huf_decoder_t **self);


// Decodes the data according to the provided configuration.
huf_error_t
huf_decode(const huf_config_t *config);


#undef CFFI_huffman_decoder_h__
#endif // INCLUDE_huffman_decoder_h__
