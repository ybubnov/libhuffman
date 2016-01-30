#ifndef INCLUDE_huffman_sys_h__
#define INCLUDE_huffman_sys_h__


#include <stdio.h>


#define void_pptr_m(pointer) ((void**)(pointer))


// Initialize the function routine.
#define routine_m() \
    huf_error_t __error = HUF_ERROR_SUCCESS \


// Set the routine finalization label.
#define routine_ensure_m() \
    ensure: \


// Leave the function routine.
#define routine_defer_m() \
    do { \
        return __error; \
    } while(0) \


// Leave the function routine without finalization.
#define routine_yield_m() \
    do { \
        routine_ensure_m(); \
        return __error; \
    } while (0) \


// Validate that provided parameter is not equal to the nil.
#define routine_param_m(param) \
    do { \
        if ((param) == 0) { \
            __error = HUF_ERROR_INVALID_ARGUMENT; \
            goto ensure; \
        } \
    } while (0) \


// Validate that specified parameter is satisfying
// criteria: low < value < high, and throw error otherwise.
#define routine_inrange_m(value, low, high) \
    do { \
        if ((value) < (low) || (value) > (high)) { \
            __error = HUF_ERROR_INVALID_ARGUMENT; \
            goto ensure; \
        } \
    } while(0) \


// Leave the function with the provided error.
#define routine_error_m(error) \
    do { \
        __error = (error); \
        goto ensure; \
    } while (0) \


// Leave the function with HUF_ERROR_SUCCESS.
#define routine_success_m() \
    routine_error_m(HUF_ERROR_SUCCESS) \


// Return true when the routine interrupted with
// an error.
#define routine_violation_m()\
    (__error != HUF_ERROR_SUCCESS) \


#endif // INCLUDE_huffman_sys_h__
