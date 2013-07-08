/* assert.h - Static assertions, panic if fail.
 *
 * Assertions will only be compiled if MINK_ASSERTIONS is defined.
 * If not, the macro will expand to ((void)0).
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
#ifndef __MINK_ASSERT_H
#define __MINK_ASSERT_H

#ifdef MINK_ASSERTIONS
#include "sys.h"

#define assert(cond) ( (cond) ? (void)0 : panic(#cond " (%s : line %d)", __FILE__, __LINE__) )

#else
#define assert(cond) ((void)0)
#endif
#endif
