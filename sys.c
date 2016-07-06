/* sys.c - Architecture-independent system-level functions for Mink.
 *
 * Copyright (c)2013-2016 Ross Bamford. See LICENSE for details.
 */
#include "sys.h"
#include "hal.h"
#include "utils.h"
#include "vsprintf.h"

void *memcpy(void *restrict s1, const void *restrict s2, size_t n) {
  char *dp = s1;
  const char *sp = s2;
  while (n--) {
    *dp++ = *sp++;
  }
  return s1;
}

void *memset(void *s, int c, size_t n) {
  unsigned char* p=s;
  while(n--) {
    *p++ = (unsigned char)c;
  }
  return s;
}

void *memsetw(void *s, int c, size_t n) {
  unsigned short* p=s;
  while(n--) {
    *p++ = (unsigned short)c;
  }
  return s;
}

noreturn void panic(const char *fmt, ...) {
  static char buf [1024];

  va_list args;
  int i;

  va_start(args, fmt);
  i = vsprintf(buf,fmt,args);
  va_end(args);

  buf[i] = '\0';

  console_setcolor(make_color(COLOR_LIGHT_RED, COLOR_BLACK));
  printk("\n\nOSHI...: %s\n", buf);
  print_stack_trace();
  printk("\n***\nSo long, and thanks for all the fish.");
  die();
}



