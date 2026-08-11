#include "jakc/assets.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

static void set_error(char* error, size_t error_cap, const char* message) {
  if (error && error_cap > 0) {
    snprintf(error, error_cap, "%s", message);
  }
}

void asset_free_buffer(ByteBuffer* buffer) {
  if (!buffer) {
    return;
  }
  free(buffer->data);
  buffer->data = NULL;
  buffer->size = 0;
}

bool asset_read_file(const char* path, ByteBuffer* out_buffer, char* error, size_t error_cap) {
  if (!path || !out_buffer) {
    set_error(error, error_cap, "invalid file read request");
    return false;
  }

  FILE* file = fopen(path, "rb");
  if (!file) {
    snprintf(error, error_cap, "failed to open %s: %s", path, strerror(errno));
    return false;
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    set_error(error, error_cap, "failed to seek file");
    return false;
  }

  long file_size = ftell(file);
  if (file_size < 0) {
    fclose(file);
    set_error(error, error_cap, "failed to measure file size");
    return false;
  }

  rewind(file);

  unsigned char* data = (unsigned char*)malloc((size_t)file_size);
  if (!data) {
    fclose(file);
    set_error(error, error_cap, "out of memory while reading file");
    return false;
  }

  size_t bytes_read = fread(data, 1, (size_t)file_size, file);
  fclose(file);
  if (bytes_read != (size_t)file_size) {
    free(data);
    set_error(error, error_cap, "failed to read full file");
    return false;
  }

  out_buffer->data = data;
  out_buffer->size = (size_t)file_size;
  return true;
}

bool asset_write_file(const char* path,
                      const unsigned char* data,
                      size_t size,
                      char* error,
                      size_t error_cap) {
  FILE* file = fopen(path, "wb");
  if (!file) {
    snprintf(error, error_cap, "failed to open %s for writing: %s", path, strerror(errno));
    return false;
  }

  size_t bytes_written = fwrite(data, 1, size, file);
  fclose(file);
  if (bytes_written != size) {
    set_error(error, error_cap, "failed to write full file");
    return false;
  }

  return true;
}

static bool mkdir_single(const char* path) {
#ifdef _WIN32
  return _mkdir(path) == 0 || errno == EEXIST;
#else
  return mkdir(path, 0777) == 0 || errno == EEXIST;
#endif
}

bool asset_mkdir_p(const char* path, char* error, size_t error_cap) {
  if (!path || !*path) {
    return true;
  }

  char temp[1024];
  size_t len = strlen(path);
  if (len >= sizeof(temp)) {
    set_error(error, error_cap, "path too long");
    return false;
  }

  memcpy(temp, path, len + 1);
  for (size_t i = 1; temp[i]; i++) {
    if (temp[i] == '/' || temp[i] == '\\') {
      char saved = temp[i];
      temp[i] = '\0';
      if (!mkdir_single(temp)) {
        snprintf(error, error_cap, "failed to create directory %s: %s", temp, strerror(errno));
        return false;
      }
      temp[i] = saved;
    }
  }

  if (!mkdir_single(temp)) {
    snprintf(error, error_cap, "failed to create directory %s: %s", temp, strerror(errno));
    return false;
  }

  return true;
}

bool asset_path_join(char* out, size_t out_cap, const char* left, const char* right) {
  if (!out || out_cap == 0) {
    return false;
  }

  const char* separator = "";
  size_t left_len = strlen(left);
  if (left_len > 0 && left[left_len - 1] != '/' && left[left_len - 1] != '\\') {
    separator = "/";
  }

  int written = snprintf(out, out_cap, "%s%s%s", left, separator, right);
  return written >= 0 && (size_t)written < out_cap;
}
