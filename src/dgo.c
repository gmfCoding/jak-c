#include "jakc/dgo.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jakc/assets.h"

typedef struct DgoHeaderRaw {
  unsigned int object_count;
  char name[60];
} DgoHeaderRaw;

typedef struct DgoObjectHeaderRaw {
  unsigned int size;
  char name[60];
} DgoObjectHeaderRaw;

static void set_error(char* error, size_t error_cap, const char* message) {
  if (error && error_cap > 0) {
    snprintf(error, error_cap, "%s", message);
  }
}

static void copy_trimmed_name(char* dst, size_t dst_cap, const char* src, size_t src_size) {
  size_t len = 0;
  while (len < src_size && src[len] != '\0') {
    len++;
  }
  if (len >= dst_cap) {
    len = dst_cap - 1;
  }
  memcpy(dst, src, len);
  dst[len] = '\0';
}

static void sanitize_name(char* dst, size_t dst_cap, const char* src) {
  size_t out = 0;
  for (size_t i = 0; src[i] != '\0' && out + 1 < dst_cap; i++) {
    unsigned char c = (unsigned char)src[i];
    if (isalnum(c) || c == '.' || c == '_' || c == '-') {
      dst[out++] = (char)c;
    } else {
      dst[out++] = '_';
    }
  }
  dst[out] = '\0';
}

bool dgo_is_compressed(const unsigned char* data, size_t size) {
  return data && size >= 4 && memcmp(data, "oZlB", 4) == 0;
}

static bool parse_archive(const char* path,
                          const unsigned char* data,
                          size_t size,
                          DgoArchive* out_archive,
                          char* error,
                          size_t error_cap) {
  if (size < sizeof(DgoHeaderRaw)) {
    set_error(error, error_cap, "file is too small to be a DGO archive");
    return false;
  }

  memset(out_archive, 0, sizeof(*out_archive));
  strncpy(out_archive->file_name, path, sizeof(out_archive->file_name) - 1);

  DgoHeaderRaw header;
  memcpy(&header, data, sizeof(header));
  copy_trimmed_name(out_archive->internal_name, sizeof(out_archive->internal_name), header.name,
                    sizeof(header.name));
  out_archive->object_count = header.object_count;

  size_t offset = sizeof(DgoHeaderRaw);
  out_archive->entries = (DgoEntry*)calloc(out_archive->object_count, sizeof(DgoEntry));
  if (!out_archive->entries) {
    set_error(error, error_cap, "out of memory while parsing DGO archive");
    return false;
  }

  for (unsigned int i = 0; i < out_archive->object_count; i++) {
    if (offset + sizeof(DgoObjectHeaderRaw) > size) {
      set_error(error, error_cap, "unexpected end of file while reading DGO object header");
      dgo_free_archive(out_archive);
      return false;
    }

    DgoObjectHeaderRaw obj_header;
    memcpy(&obj_header, data + offset, sizeof(obj_header));
    offset += sizeof(DgoObjectHeaderRaw);

    if (offset + obj_header.size > size) {
      set_error(error, error_cap, "unexpected end of file while reading DGO object payload");
      dgo_free_archive(out_archive);
      return false;
    }

    DgoEntry* entry = &out_archive->entries[i];
    copy_trimmed_name(entry->internal_name, sizeof(entry->internal_name), obj_header.name,
                      sizeof(obj_header.name));
    entry->size = obj_header.size;
    entry->data = data + offset;

    char safe_name[192];
    sanitize_name(safe_name, sizeof(safe_name), entry->internal_name);
    snprintf(entry->output_name, sizeof(entry->output_name), "%04u_%s.bin", i, safe_name);

    offset += obj_header.size;
    while (offset % 16 != 0) {
      offset++;
    }
  }

  return true;
}

bool dgo_load_archive(const char* path, DgoArchive* out_archive, char* error, size_t error_cap) {
  ByteBuffer buffer = {0};
  if (!asset_read_file(path, &buffer, error, error_cap)) {
    return false;
  }

  if (dgo_is_compressed(buffer.data, buffer.size)) {
    asset_free_buffer(&buffer);
    set_error(error, error_cap, "compressed DGOs are not supported yet");
    return false;
  }

  bool ok = parse_archive(path, buffer.data, buffer.size, out_archive, error, error_cap);
  if (!ok) {
    asset_free_buffer(&buffer);
    return false;
  }

  return true;
}

void dgo_free_archive(DgoArchive* archive) {
  if (!archive) {
    return;
  }
  free(archive->entries);
  archive->entries = NULL;
  archive->object_count = 0;
  archive->file_name[0] = '\0';
  archive->internal_name[0] = '\0';
}

bool dgo_summarize_archive(const DgoArchive* archive, char* out, size_t out_cap) {
  if (!archive || !out || out_cap == 0) {
    return false;
  }

  size_t used = (size_t)snprintf(out, out_cap,
                                  "DGO: %s\nPackage: %s\nObjects: %u\n",
                                  archive->file_name, archive->internal_name,
                                  archive->object_count);
  if (used >= out_cap) {
    return false;
  }

  for (unsigned int i = 0; i < archive->object_count; i++) {
    const DgoEntry* entry = &archive->entries[i];
    int written = snprintf(out + used, out_cap - used, "  [%u] %s (%u bytes)\n", i,
                           entry->internal_name, entry->size);
    if (written < 0 || (size_t)written >= out_cap - used) {
      return false;
    }
    used += (size_t)written;
  }

  return true;
}

bool dgo_unpack_archive(const DgoArchive* archive,
                        const char* output_dir,
                        char* error,
                        size_t error_cap) {
  if (!archive || !output_dir) {
    set_error(error, error_cap, "invalid unpack request");
    return false;
  }

  if (!asset_mkdir_p(output_dir, error, error_cap)) {
    return false;
  }

  char bundle_dir[1024];
  if (!asset_path_join(bundle_dir, sizeof(bundle_dir), output_dir, archive->internal_name)) {
    set_error(error, error_cap, "bundle output path is too long");
    return false;
  }

  if (!asset_mkdir_p(bundle_dir, error, error_cap)) {
    return false;
  }

  for (unsigned int i = 0; i < archive->object_count; i++) {
    char file_path[1024];
    if (!asset_path_join(file_path, sizeof(file_path), bundle_dir, archive->entries[i].output_name)) {
      set_error(error, error_cap, "object output path is too long");
      return false;
    }
    if (!asset_write_file(file_path, archive->entries[i].data, archive->entries[i].size, error,
                          error_cap)) {
      return false;
    }
  }

  return true;
}
