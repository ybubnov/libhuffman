#ifndef RT_H
#define RT_H

#define __TRY__ \
    huf_error_t __error__ = HUF_ERROR_SUCCESS \

#define __FINALLY__\
    __finally__: \

#define __END__ \
    do { \
        return __error__; \
    } while(0) \

#define __RAISED__ \
    (__error__ != HUF_ERROR_SUCCESS) \

#define __RAISE__(error) \
    do { \
        __error__ = (error); \
        goto __finally__; \
    } while(0) \

#define __ASSERT__(statement, error) \
    do { \
        if ((statement) != HUF_ERROR_SUCCESS) { \
            __error__ = (error); \
            goto __finally__; \
        } \
    } while(0) \

#define __ASSERT_NULL__(statement, error) \
    do { \
        if ((statement) == 0) { \
            __error__ = (error); \
            goto __finally__; \
        } \
    } while(0) \

#define __ASSERT_TRUE__(statement, error) \
    do { \
        if ((statement)) { \
            __error__ = (error); \
            goto __finally__; \
        } \
    } while(0) \

#endif // RT_H
