/*
 * mink_ramfs.c
 *
 *  Created on: 4 Aug 2016
 *      Author: Ross Bamford <roscopeco AT gmail DOT com>
 */

#include <stddef.h>
#include "ramfs.h"

bool is_valid_ramfs(void *mem) {
  if (mem == NULL) {
    return false;
  }

  uint32_t magic = *((uint32_t*)mem);
  if (magic != MINK_RAMFS_MAGIC) {
    return false;
  }

  // looks like a ramfs at this point, so lets go for the size...
  if (*((uint32_t*)mem+1) != MINK_RAMFS_VERSION) {
    return false;
  }

  return true;
}

int ramfs_size(mink_ramfs_header_t *fs) {
  if (!is_valid_ramfs(fs))
      return -1;

  return fs->fs_size;
}

int ramfs_file_count(mink_ramfs_header_t *fs) {
  if (!is_valid_ramfs(fs))
      return -1;

  return fs->file_count;
}

mink_ramfs_file_header_t* ramfs_find_file(mink_ramfs_header_t *fs, char *name) {
  if (!is_valid_ramfs(fs))
      return NULL;

  if (name == NULL)
      return NULL;

  // first file is at end of fs header
  mink_ramfs_file_header_t* current = (mink_ramfs_file_header_t*)(fs + 1);

  for (int i = 0; i < fs->file_count; i++, current++) {
    if (current->file_name != NULL &&
        strncmp(name, current->file_name, 24) == 0) {
      return current;
    }
  }

  return NULL;
}

void* ramfs_file_open(mink_ramfs_file_header_t *file) {
  if (file == NULL)
      return NULL;

  if (file->file_length == 0)
      return NULL;

  if (file->file_start == 0)
      return NULL;

  return (void*)(((char*)file) + file->file_start);
}
