/**
 * @file mpegaudiodectab.h
 * mpeg audio layer decoder tables.
 */


#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>

#include <math.h>
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
#include <stddef.h>
#ifndef offsetof
# define offsetof(T,F) ((unsigned int)((char *)&((T *)0)->F))
#endif
#ifndef EMULATE_INTTYPES
#   include <inttypes.h>
#else
    typedef signed char  int8_t;
    typedef signed short int16_t;
    typedef signed int   int32_t;
    typedef unsigned char  uint8_t;
    typedef unsigned short uint16_t;
    typedef unsigned int   uint32_t;

#   ifdef CONFIG_WIN32
        typedef signed __int64   int64_t;
        typedef unsigned __int64 uint64_t;
#   else /* other OS */
        typedef signed int64_t int64_t   int64_t;
        typedef unsigned int64_t int64_t uint64_t;
#   endif /* other OS */
#endif /* HAVE_INTTYPES_H */

#define int64_t_C(c)     (c ## LL)
#define uint64_t_C(c)    (c ## ULL)


#ifndef restrict
#    define restrict
#endif

#ifndef always_inline
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define always_inline __attribute__((always_inline)) inline
#else
#    define always_inline inline
#endif
#endif

#ifndef attribute_used
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define attribute_used __attribute__((used))
#else
#    define attribute_used
#endif
#endif

#ifndef attribute_unused
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define attribute_unused __attribute__((unused))
#else
#    define attribute_unused
#endif
#endif


 
#ifndef __MPEGAUDIODECTAB_H__
#define __MPEGAUDIODECTAB_H__

/* encoding tables which give the quantization index. Note how it is
   possible to store them efficiently ! */
static const unsigned char alloc_table_0[] = {
 4,  0,  2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
 4,  0,  2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
 4,  0,  2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 2,  0,  1, 16,
 2,  0,  1, 16,
 2,  0,  1, 16,
 2,  0,  1, 16,
};

static const unsigned char alloc_table_1[] = {
 4,  0,  2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
 4,  0,  2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
 4,  0,  2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 3,  0,  1,  2,  3,  4,  5, 16,
 2,  0,  1, 16,
 2,  0,  1, 16,
 2,  0,  1, 16,
 2,  0,  1, 16,
 2,  0,  1, 16,
 2,  0,  1, 16,
 2,  0,  1, 16,
};

static const unsigned char alloc_table_2[] = {
 4,  0,  1,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
 4,  0,  1,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
};

static const unsigned char alloc_table_3[] = {
 4,  0,  1,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
 4,  0,  1,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
};

static const unsigned char alloc_table_4[] = {
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
 4,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 3,  0,  1,  3,  4,  5,  6,  7,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
 2,  0,  1,  3,
};

#endif



