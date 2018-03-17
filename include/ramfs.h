/*
 * ramfs.h - Initial ramfs for Mink kernel.
 *
 * The initial ramfs is a small filesystem loaded into RAM by the
 * bootloader. It holds the basic subset of drivers and servers required
 * to initiate the rest of the boot process.
 *
 * As it stands, it's a stretch to even call this a filesystem. It's basically
 * just a block of memory that has named blocks within it (i.e the files).
 * When a file is "opened", you literally just get back a pointer to the
 * start of that files data - it's up to you to make sure you don't
 * read past it, corrupt it, or whatever...
 *
 * The idea is that grub will load a flat copy of one of these "filesystems"
 * as a module, and the kernel will early_alloc some space for it and keep
 * it hanging around somewhere, mapped into kernel space, and the init processes
 * will run directly from there.
 *
 * The kernel itself has no concept of "File" - that will all be handled
 * by the actual FS servers.
 *
 *  Created on: 4 Aug 2016
 *      Author: Ross Bamford <roscopeco AT gmail DOT com>
  *
 * Copyright (c)2016-2018 Ross Bamford. See LICENSE for details.
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

/*
 * Checks that the memory pointed to by mem is a valid ramfs.
 * This includes a magic check and a version check.
 */
bool is_valid_ramfs(void *mem);

/*
 * Gets the size of the ramfs, as reported by the header.
 * Returns -1 if the fs is not valid.
 */
int ramfs_size(mink_ramfs_header_t *fs);

/*
 * Gets the number of files in the ramfs, as reported by the header.
 * Returns -1 if the fs is not valid.
 */
int ramfs_file_count(mink_ramfs_header_t *fs);

/*
 * Finds the named file from the given fs.
 * Returns NULL if the fs is not valid, or if the name is NULL.
 */
mink_ramfs_file_header_t* ramfs_find_file(mink_ramfs_header_t *fs, char *name);

/*
 * 'Open' the given file. Just returns a pointer to the start of the data.
 * Returns NULL if the given file is NULL or empty.
 */
void* ramfs_file_open(mink_ramfs_file_header_t *file);

#endif /* MINK_RAMFS_H_ */
