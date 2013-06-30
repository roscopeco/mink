/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */
#ifndef __MINK_VSPRINTF_H
#define __MINK_VSPRINTF_H

#include <stdarg.h>

int vsprintf(char *buf, const char *fmt, va_list args);

#endif

