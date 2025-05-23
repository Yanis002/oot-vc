#ifndef _MEM_FUNCS_H
#define _MEM_FUNCS_H

#include "macros.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __min_bytes_for_long_copy 32

INIT void* memcpy(void* dst, const void* src, size_t n);
INIT void* memset(void* dst, int val, size_t n);
INIT void __fill_mem(void* dst, int val, size_t n);
void* memmove(void*, const void*, size_t);

void __copy_mem(void* dst, const void* src, unsigned long n);
void __move_mem(void* dst, const void* src, unsigned long n);
void __copy_longs_aligned(void* dst, const void* src, size_t len);
void __copy_longs_rev_aligned(void* dst, const void* src, size_t len);
void __copy_longs_unaligned(void* dst, const void* src, size_t len);
void __copy_longs_rev_unaligned(void* pDest, const void* pSrc, size_t len);

#ifdef __cplusplus
}
#endif

#endif
