#ifndef LIBC_H
#define LIBC_H

#include <stddef.h>

#if defined(STM32H7) && !defined(PANDA_HOST_TEST)
  // Firmware build: Use our custom implementation
  // cppcheck-suppress misra-c2012-21.2
  void *memset(void *str, int c, size_t n);
  // cppcheck-suppress misra-c2012-21.2
  void *memcpy(void *dest, const void *src, size_t n);
  // cppcheck-suppress misra-c2012-21.2
  int memcmp(const void *s1, const void *s2, size_t n);
#else
  // Host test build (macOS/Ubuntu): Use system headers ONLY
  #include <string.h>
#endif

#endif
