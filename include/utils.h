/* utils.h - Utility functions for Mink.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#ifndef __MINK_UTILS_H
#define __MINK_UTILS_H
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

/**
 * Like printf, but for the kernel.
 */
void printk(const char *fmt, ...);

/**
 * Like printk, but for debugging.
 * 
 * TODO remove this...
 */
void debugk(const char *fmt, ...);

/**
 * Should behave exactly like the standard.
 */
size_t strlen(const char *str);

/**
 * Should behave exactly like the standard.
 */
int strcmp(const char* s1, const char* s2);

/**
 * Tokenize 'in' using token character 'tok', placing at most 'maxout' tokens
 * in 'out'.
 */
int tokenize(char tok, char *in, const char **out, int maxout);

unsigned log2_roundup(unsigned n);

#endif
 

