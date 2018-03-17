/* Unit tests for the ramfs subsystem.
 *
 * Copyright (c)2018 Ross Bamford. See LICENSE for details.
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "ramfs.c"

static mink_ramfs_header_t* create_testmodel_no_files(int version) {
  mink_ramfs_header_t *model = calloc(1, sizeof(mink_ramfs_header_t));

  model->magic = MINK_RAMFS_MAGIC;
  model->version = version;
  model->file_count = 0;
  model->fs_size = sizeof(mink_ramfs_header_t);

  return model;
}

static mink_ramfs_header_t* create_testmodel_one_file(int version) {
  char *content = "File content";
  mink_ramfs_header_t *model = calloc(1, sizeof(mink_ramfs_header_t) + sizeof(mink_ramfs_file_header_t) + strlen(content) + 1);
  mink_ramfs_file_header_t *file = (mink_ramfs_file_header_t*)(model + 1);
  char *filedata = (char*)(file + 1);

  model->magic = MINK_RAMFS_MAGIC;
  model->version = version;
  model->file_count = 1;
  model->fs_size = sizeof(mink_ramfs_header_t);

  file->file_length = strlen(content) + 1;
  strcpy(file->file_name, "file001");
  file->file_start = sizeof(mink_ramfs_file_header_t);

  strcpy(filedata, content);

  return model;
}

static mink_ramfs_header_t* create_testmodel_one_file_malformed(int version) {
  char *content1 = "File 1ontent";
  mink_ramfs_header_t *model = calloc(1, sizeof(mink_ramfs_header_t) + sizeof(mink_ramfs_file_header_t) + strlen(content1) + 1);
  mink_ramfs_file_header_t *file = (mink_ramfs_file_header_t*)(model + 1);
  char *filedata = (char*)(file + 1);

  model->magic = MINK_RAMFS_MAGIC;
  model->version = version;
  model->file_count = 1;
  model->fs_size = sizeof(mink_ramfs_header_t);

  file->file_length = strlen(content1) + 1;
  memcpy(file->file_name, "123456789^123456789^1234", 24);          // MALFORMED - NOT NULL TERMINATED
  file->file_start = sizeof(mink_ramfs_file_header_t);

  strcpy(filedata, content1);

  return model;
}

static mink_ramfs_header_t* create_testmodel_three_files(int version) {
  char *content1 = "File 1ontent";      // these must stay same length!
  char *content2 = "File 2ontent";

  mink_ramfs_header_t *model = calloc(1, sizeof(mink_ramfs_header_t) + (sizeof(mink_ramfs_file_header_t) * 3) + (strlen(content1) * 2) + 2);

  model->magic = MINK_RAMFS_MAGIC;
  model->version = version;
  model->file_count = 2;
  model->fs_size = sizeof(mink_ramfs_header_t);

  mink_ramfs_file_header_t *file = (mink_ramfs_file_header_t*)(model + 1);
  file->file_length = strlen(content1) + 1;
  strcpy(file->file_name, "file001");
  file->file_start = sizeof(mink_ramfs_file_header_t) * 3;

  file++;
  file->file_length = strlen(content2) + 1;
  strcpy(file->file_name, "file002");
  file->file_start = sizeof(mink_ramfs_file_header_t) * 2 + strlen(content1) + 1;

  file++;
  file->file_length = 0;
  strcpy(file->file_name, "file003_empty");
  file->file_start = 0;

  char *filedata = (char*)(file + 1);
  strcpy(filedata, content1);

  filedata = filedata + strlen(content1) + 1;
  strcpy(filedata, content2);

  return model;
}

void test_is_valid_ramfs_with_null_ramfs() {
  void* null_ramfs = NULL;
  TEST_ASSERT_FALSE(is_valid_ramfs(null_ramfs));
}

void test_is_valid_ramfs_with_invalid_ramfs() {
  void *invalid_ramfs = calloc(1, sizeof(mink_ramfs_header_t));

  TEST_ASSERT_FALSE(is_valid_ramfs(invalid_ramfs));

  free(invalid_ramfs);
}

void test_is_valid_ramfs_with_incorrect_version() {
  mink_ramfs_header_t *invalid_ramfs = create_testmodel_no_files(MINK_RAMFS_VERSION + 10);

  TEST_ASSERT_FALSE(is_valid_ramfs(invalid_ramfs));

  free(invalid_ramfs);
}

void test_is_valid_ramfs_with_valid_ramfs() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_no_files(MINK_RAMFS_VERSION);

  TEST_ASSERT_TRUE(is_valid_ramfs(valid_ramfs));

  free(valid_ramfs);
}

void test_ramfs_size_with_null_ramfs() {
  void* null_ramfs = NULL;
  TEST_ASSERT_EQUAL_INT(-1, ramfs_size(null_ramfs));
}

void test_ramfs_size_with_invalid_ramfs() {
  void *invalid_ramfs = calloc(1, sizeof(mink_ramfs_header_t));

  TEST_ASSERT_EQUAL_INT(-1, ramfs_size(invalid_ramfs));

  free(invalid_ramfs);
}

void test_ramfs_size_with_incorrect_version() {
  mink_ramfs_header_t *invalid_ramfs = create_testmodel_no_files(MINK_RAMFS_VERSION + 5);

  TEST_ASSERT_EQUAL_INT(-1, ramfs_size(invalid_ramfs));

  free(invalid_ramfs);
}

void test_ramfs_size_with_valid_ramfs() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_no_files(MINK_RAMFS_VERSION);

  TEST_ASSERT_EQUAL_INT(sizeof(mink_ramfs_header_t), ramfs_size(valid_ramfs));

  free(valid_ramfs);
}

void test_ramfs_file_count_with_null_ramfs() {
  void* null_ramfs = NULL;
  TEST_ASSERT_EQUAL_INT(-1, ramfs_file_count(null_ramfs));
}

void test_ramfs_file_count_with_invalid_ramfs() {
  void *invalid_ramfs = calloc(1, sizeof(mink_ramfs_header_t));

  TEST_ASSERT_EQUAL_INT(-1, ramfs_file_count(invalid_ramfs));

  free(invalid_ramfs);
}

void test_ramfs_file_count_with_incorrect_version() {
  mink_ramfs_header_t *invalid_ramfs = create_testmodel_no_files(MINK_RAMFS_VERSION + 5);

  TEST_ASSERT_EQUAL_INT(-1, ramfs_file_count(invalid_ramfs));

  free(invalid_ramfs);
}

void test_ramfs_file_count_with_valid_empty_ramfs() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_no_files(MINK_RAMFS_VERSION);

  TEST_ASSERT_EQUAL_INT(0, ramfs_file_count(valid_ramfs));

  free(valid_ramfs);
}

void test_ramfs_file_count_with_valid_ramfs_and_one_file() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_one_file(MINK_RAMFS_VERSION);

  TEST_ASSERT_EQUAL_INT(1, ramfs_file_count(valid_ramfs));

  free(valid_ramfs);
}

void test_ramfs_find_file_with_null_ramfs() {
  void* null_ramfs = NULL;
  TEST_ASSERT_NULL(ramfs_find_file(null_ramfs, "file001"));
}

void test_ramfs_find_file_with_invalid_ramfs() {
  void *invalid_ramfs = calloc(1, sizeof(mink_ramfs_header_t));

  TEST_ASSERT_NULL(ramfs_find_file(invalid_ramfs, "file001"));

  free(invalid_ramfs);
}

void test_ramfs_find_file_with_incorrect_version() {
  mink_ramfs_header_t *invalid_ramfs = create_testmodel_no_files(MINK_RAMFS_VERSION + 5);

  TEST_ASSERT_NULL(ramfs_find_file(invalid_ramfs, "file001"));

  free(invalid_ramfs);
}

void test_ramfs_find_file_with_valid_empty_ramfs() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_no_files(MINK_RAMFS_VERSION);

  TEST_ASSERT_NULL(ramfs_find_file(valid_ramfs, "file001"));

  free(valid_ramfs);
}

void test_ramfs_find_file_with_valid_empty_ramfs_but_null_filename() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_no_files(MINK_RAMFS_VERSION);

  TEST_ASSERT_NULL(ramfs_find_file(valid_ramfs, NULL));

  free(valid_ramfs);
}

void test_ramfs_find_file_with_valid_ramfs_and_one_file_but_not_found() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_one_file(MINK_RAMFS_VERSION);

  TEST_ASSERT_NULL(ramfs_find_file(valid_ramfs, "filenotfound"));

  free(valid_ramfs);
}

void test_ramfs_find_file_with_valid_ramfs_and_one_file_but_null_filename() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_one_file(MINK_RAMFS_VERSION);

  TEST_ASSERT_NULL(ramfs_find_file(valid_ramfs, NULL));

  free(valid_ramfs);
}

void test_ramfs_find_file_with_valid_ramfs_and_one_file_can_handle_malformed_ramfs() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_one_file_malformed(MINK_RAMFS_VERSION);

  TEST_ASSERT_NULL(ramfs_find_file(valid_ramfs, "file001"));

  free(valid_ramfs);
}

void test_ramfs_find_file_with_valid_ramfs_and_one_file_that_exists() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_one_file(MINK_RAMFS_VERSION);
  mink_ramfs_file_header_t *file = ramfs_find_file(valid_ramfs, "file001");

  TEST_ASSERT_NOT_NULL(file);
  TEST_ASSERT_EQUAL_STRING("file001", file->file_name);
  TEST_ASSERT_EQUAL_INT(strlen("File content") + 1, file->file_length);

  free(valid_ramfs);
}

void test_ramfs_find_file_with_valid_ramfs_and_two_files_that_exists() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_three_files(MINK_RAMFS_VERSION);

  mink_ramfs_file_header_t *file = ramfs_find_file(valid_ramfs, "file001");
  TEST_ASSERT_NOT_NULL(file);
  TEST_ASSERT_EQUAL_STRING("file001", file->file_name);
  TEST_ASSERT_EQUAL_INT(strlen("File 1ontent") + 1, file->file_length);

  file = ramfs_find_file(valid_ramfs, "file002");
  TEST_ASSERT_NOT_NULL(file);
  TEST_ASSERT_EQUAL_STRING("file002", file->file_name);
  TEST_ASSERT_EQUAL_INT(strlen("File 2ontent") + 1, file->file_length);

  free(valid_ramfs);
}

void test_ramfs_file_open_with_null_file() {
  mink_ramfs_file_header_t* null_file = NULL;
  TEST_ASSERT_NULL(ramfs_file_open(null_file));
}

void test_ramfs_file_open_with_empty_file() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_three_files(MINK_RAMFS_VERSION);
  mink_ramfs_file_header_t *file = ramfs_find_file(valid_ramfs, "file003_empty");

  TEST_ASSERT_NULL(ramfs_file_open(file));

  free(valid_ramfs);
}

void test_ramfs_file_open_with_normal_file() {
  mink_ramfs_header_t *valid_ramfs = create_testmodel_three_files(MINK_RAMFS_VERSION);
  mink_ramfs_file_header_t *file1 = ramfs_find_file(valid_ramfs, "file001");
  mink_ramfs_file_header_t *file2 = ramfs_find_file(valid_ramfs, "file002");

  char *data1 = (char*)ramfs_file_open(file1);
  char *data2 = (char*)ramfs_file_open(file2);

  TEST_ASSERT_NOT_NULL(data1);
  TEST_ASSERT_EQUAL_STRING("File 1ontent", data1);

  TEST_ASSERT_NOT_NULL(data2);
  TEST_ASSERT_EQUAL_STRING("File 2ontent", data2);

  free(valid_ramfs);
}
