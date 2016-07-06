/* Mink - A Microkernel.
 *
 * This is open-source software, licensed under the MIT license.
 * See LICENSE for more details.
 *
 * Copyright (c)2013 Ross Bamford.
 */

#ifndef __MINK_H
#define __MINK_H

#ifdef __MINK_KERNEL__
#if defined(__linux__)
#error "Will not compile with platform CC. Please use a cross compiler - see http://wiki.osdev.org/GCC_Cross-Compiler"
#endif
#endif

#include "hal.h"
#include "sys.h"
#include "utils.h"

#define MINK_VERSION "0.1.1"

#endif
