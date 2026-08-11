#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct DgoEntry {
  char internal_name[61];
  char output_name[256];
  unsigned int size;
  const unsigned char* data;
} DgoEntry;

typedef struct DgoArchive {
  char file_name[256];
  char internal_name[61];
  unsigned int object_count;
  DgoEntry* entries;
} DgoArchive;

bool dgo_is_compressed(const unsigned char* data, size_t size);
bool dgo_load_archive(const char* path, DgoArchive* out_archive, char* error, size_t error_cap);
void dgo_free_archive(DgoArchive* archive);
bool dgo_unpack_archive(const DgoArchive* archive,
                        const char* output_dir,
                        char* error,
                        size_t error_cap);
bool dgo_summarize_archive(const DgoArchive* archive, char* out, size_t out_cap);
