/* utils.c - Utility functions for Mink.
 *
 * Portions based on code from http://wiki.osdev.org/Bare_Bones#Writing_a_kernel_in_C
 * Portions based on code from JamesM's tutorials.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "hal.h"
#include "sys.h"
#include "vsprintf.h"
#include "elf.h"

extern bool KDEBUG;

size_t strlen(const char* str) {
	size_t ret = 0;
	while ( str[ret] != 0 )
		ret++;
	return ret;
}

int strcmp(const char* s1, const char* s2) {
  while(*s1 && (*s1==*s2))
      s1++,s2++;
  return *(const unsigned char*)s1-*(const unsigned char*)s2;
}

void delay(int loops) {
  for (int i = 0; i < loops; i++) {
    int a = i * 7;
    a = a / 3;
  }
}

void printk(const char *fmt, ...) {
  static char buf [1024];

  va_list args;
  int i;

  va_start(args, fmt);
  i = vsprintf(buf,fmt,args);
  va_end(args);

  buf[i] = '\0';

  console_writestring(buf);
}

void debugk(const char *fmt, ...) {
  if (KDEBUG) {
    static char buf [1024];

    va_list args;
    int i;

    va_start(args, fmt);
    i = vsprintf(buf,fmt,args);
    va_end(args);

    buf[i] = '\0';

    console_writestring(buf);
  }
}

int tokenize(char tok, char *in, const char **out, int maxout) {
  int n = 0;

  while(*in && n < maxout) {
    out[n++] = in;

    /* Spool until the next instance of 'tok', or end of string. */
    while (*in && *in != tok)
      ++in;
    /* If we exited because we saw a token, make it a NUL character
       and step over it.*/
    if (*in == tok)
      *in++ = '\0';
  }

  return n;
}

