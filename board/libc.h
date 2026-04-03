#ifndef LIBC_H
#define LIBC_H

#include <stddef.h>

// cppcheck-suppress misra-c2012-21.2 ; custom libc implementation
void *memset(void *str, int c, size_t n);
// cppcheck-suppress misra-c2012-21.2 ; custom libc implementation
void *memcpy(void *dest, const void *src, size_t n);
// cppcheck-suppress misra-c2012-21.2 ; custom libc implementation
int memcmp(const void *s1, const void *s2, size_t n);

#endif
