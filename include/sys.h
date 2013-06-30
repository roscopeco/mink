/* sys.h - System-level functions for Mink Kernel.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
 
#ifndef _MINK_SYS_H
#define _MINK_SYS_H

#include <stdint.h>
#include <stddef.h>

/**
 * Should behave exactly like the standard.
 */
void *memcpy(void *restrict s1, const void *restrict s2, size_t n);

/**
 * Should behave exactly like the standard.
 */
void *memset(void *s, int c, size_t n);

/**
 * Should behave exactly like the standard, but with 16-bit words.
 */
void *memsetw(void *s, int c, size_t n);

/**
 * Issue a kernel oshi (panic). This will print the specified message, 
 * dump a stack trace, and then call die().
 */
void panic(const char *msg, ...);

#endif
