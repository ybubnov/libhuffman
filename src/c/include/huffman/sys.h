#ifndef INCLUDE_huffman_sys_h__
#define INCLUDE_huffman_sys_h__


#define __try__ \
    huf_error_t __error__ = HUF_ERROR_SUCCESS \


#define __finally__ \
    cleanup: \


#define __end__ \
    do { \
        return __error__; \
    } while (0) \


#define __raised__ \
    (__error__ != HUF_ERROR_SUCCESS) \


#define __raise__(error) \
    do { \
        __error__ = (error); \
        goto cleanup; \
    } while (0) \


#define __argument__(argument) \
    do { \
        if ((argument) == 0) { \
            __error__ = HUF_ERROR_INVALID_ARGUMENT; \
            goto cleanup; \
        } \
    } while (0) \


#define __assert__(error) \
    do { \
        if ((error) != HUF_ERROR_SUCCESS) { \
            __error__ = (error); \
            goto cleanup; \
        } \
    } while (0) \


#endif // INCLUDE_huffman_sys_h__
