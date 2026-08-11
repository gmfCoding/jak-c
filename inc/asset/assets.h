#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct ByteBuffer {
  unsigned char* data;
  size_t size;
} ByteBuffer;

bool asset_read_file(const char* path, ByteBuffer* out_buffer, char* error, size_t error_cap);
bool asset_write_file(const char* path,
                      const unsigned char* data,
                      size_t size,
                      char* error,
                      size_t error_cap);
bool asset_mkdir_p(const char* path, char* error, size_t error_cap);
bool asset_path_join(char* out, size_t out_cap, const char* left, const char* right);
void asset_free_buffer(ByteBuffer* buffer);
