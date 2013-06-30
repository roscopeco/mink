/* pmm.h - Physical memory manager for FirstOS.
 *
 * This is the basic memory allocator in FirstOS, with responsibility for 
 * managing pages of physical memory. It provides the ability to register
 * a set of usable RAM pages, and later to allocate these pages for use
 * and to free them (return them to the pool of available pages).
 *
 * Additionally, it supports a 'static' allocation mode, which is *only*
 * used before the VMM is available (i.e. before the CPU is in paging mode).
 * In this mode pages are allocated from a base memory address (set in the 
 * pmm_init call), and are never freed. This is used to allocate pages for 
 * VMM structures during VMM initialisation.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#ifndef __FIRSTOS_PMM_H
#define __FIRSTOS_PMM_H

#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <stddef.h>
#include <stdint.h>

#define PMM_STACK_BASE 0xFF000000

/**
 * Initialize the PMM. 
 *
 * Args:
 *  static_memstart         Base address for static allocation (before VMM is available)
 */
void pmm_init(uint32_t static_memstart);

/**
 * Add _page_ to the list of pages managed by the PMM. Prior to any dynamic
 * allocation, this function must be called for each usable RAM page the 
 * PMM may allocate.
 *
 * Args:
 *  page                    The page to register with the allocator
 */
void pmm_page_manage(uint32_t page);

/**
 * Determine the total number of pages registered with the allocator. This
 * includes both free pages and currently allocated pages.
 */
uint32_t pmm_get_page_count();

/**
 * Determine the number of free pages currently available to the allocator. 
 * This includes both free pages and currently allocated pages.
 */
uint32_t pmm_get_free_count();

/**
 * Allocate a page from the free page pool. This page remains the property of
 * the caller until such time as it is returned to the pool via a call to
 * pmm_page_free.
 *
 * Returns:
 *  The physical address of the allocated page.
 */
uint32_t pmm_page_alloc();

/**
 * Free the specified page and return it to the free page pool. The page will
 * be available for future allocation (via the pmm_page_alloc function).
 *
 * Args:
 *  page                    The physical address of the page to free.
 */
void pmm_page_free(uint32_t page);

#endif
