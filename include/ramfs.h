/*
 * mink_ramfs.h - Initial ramfs for Mink kernel.
 *
 * The initial ramfs is a small filesystem loaded into RAM by the
 * bootloader. It holds the basic subset of drivers and servers required
 * to initiate the rest of the boot process.
 *
 *  Created on: 4 Aug 2016
 *      Author: Ross Bamford <roscopeco AT gmail DOT com>
 */

#ifndef MINK_RAMFS_H_
#define MINK_RAMFS_H_

#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

#define MINK_RAMFS_MAGIC   0x0101CA75
#define MINK_RAMFS_VERSION 10

typedef struct __attribute__((packed)) {
  uint32_t magic;       /* 0x0101CA75 */
  uint32_t version;     /* Must equal MINK_RAMFS_VERSION */
  uint32_t fs_size;     /* Size of this filesystem. Must be a multiple of 4k */
  uint32_t file_count;  /* Number of mink_ramfs_file_header_t following header */
} mink_ramfs_header_t;

typedef struct __attribute__((packed)) {
  uint32_t file_start;  /* offset from this header to start of data */
  uint32_t file_length; /* size of file, in bytes. */
  char file_name[24];   /* File-name, null-terminated. */
} mink_ramfs_file_header_t;

bool is_valid_ramfs(void *mem);
int ramfs_size(mink_ramfs_header_t *fs);
int ramfs_file_count(mink_ramfs_header_t *fs);

mink_ramfs_file_header_t* ramfs_find_file(mink_ramfs_header_t *fs, char *name);
void* ramfs_file_open(mink_ramfs_file_header_t *file);

#endif /* MINK_RAMFS_H_ */
