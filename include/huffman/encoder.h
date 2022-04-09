#ifndef INCLUDE_huffman_encoder_h__
#define INCLUDE_huffman_encoder_h__

#include "huffman/config.h"
#include "huffman/common.h"
#include "huffman/errors.h"

#define CFFI_huffman_encoder_h__

// A Huffman encoding context.
typedef struct __huf_encoder huf_encoder_t;


// Create a new instance of the Huffman encoder.
huf_error_t
huf_encoder_init(huf_encoder_t **self, const huf_config_t *config);


// Release memory occupied by the Huffman encoder.
huf_error_t
huf_encoder_free(huf_encoder_t **self);


// Encode the data according to the provided configuration.
huf_error_t
huf_encode(const huf_config_t *config);


#undef CFFI_huffman_encoder_h__
#endif // INCLUDE_huffman_encoder_h__
