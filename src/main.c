#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

#include "jakc/assets.h"
#include "jakc/dgo.h"
#include "jakc/renderer.h"

static void print_usage(const char* program_name) {
  printf("usage:\n");
  printf("  %s [--assets <dir>] [--unpack <bundle> <output-dir>] [--width <px>] [--height <px>]\n",
         program_name);
  printf("\n");
  printf("examples:\n");
  printf("  %s --assets ../../open-goal/iso_data/jak1 --unpack ../../open-goal/iso_data/jak1/CGO/ENGINE.CGO out/engine\n",
         program_name);
  printf("  %s --width 1280 --height 720\n", program_name);
}

static const char* next_arg(int* index, int argc, char** argv) {
  if (*index + 1 >= argc) {
    return NULL;
  }
  (*index)++;
  return argv[*index];
}

static int unpack_bundle(const char* bundle_path, const char* output_dir) {
  char error[512] = {0};
  DgoArchive archive = {0};

  if (!dgo_load_archive(bundle_path, &archive, error, sizeof(error))) {
    fprintf(stderr, "failed to load bundle %s: %s\n", bundle_path, error);
    return 1;
  }

  char summary[4096] = {0};
  if (dgo_summarize_archive(&archive, summary, sizeof(summary))) {
    printf("%s", summary);
  }

  if (!dgo_unpack_archive(&archive, output_dir, error, sizeof(error))) {
    fprintf(stderr, "failed to unpack bundle %s: %s\n", bundle_path, error);
    dgo_free_archive(&archive);
    return 1;
  }

  printf("unpacked %u objects from %s into %s\n", archive.object_count, bundle_path, output_dir);
  dgo_free_archive(&archive);
  return 0;
}

int main(int argc, char** argv) {
  const char* assets_dir = NULL;
  const char* unpack_bundle_path = NULL;
  const char* unpack_output_dir = NULL;
  int width = 1280;
  int height = 720;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      print_usage(argv[0]);
      return 0;
    }
    if (strcmp(argv[i], "--assets") == 0) {
      assets_dir = next_arg(&i, argc, argv);
      continue;
    }
    if (strcmp(argv[i], "--unpack") == 0) {
      unpack_bundle_path = next_arg(&i, argc, argv);
      unpack_output_dir = next_arg(&i, argc, argv);
      continue;
    }
    if (strcmp(argv[i], "--width") == 0) {
      const char* value = next_arg(&i, argc, argv);
      if (!value) {
        fprintf(stderr, "--width requires a value\n");
        return 1;
      }
      width = atoi(value);
      continue;
    }
    if (strcmp(argv[i], "--height") == 0) {
      const char* value = next_arg(&i, argc, argv);
      if (!value) {
        fprintf(stderr, "--height requires a value\n");
        return 1;
      }
      height = atoi(value);
      continue;
    }

    fprintf(stderr, "unknown argument: %s\n", argv[i]);
    print_usage(argv[0]);
    return 1;
  }

  if (unpack_bundle_path && unpack_output_dir) {
    return unpack_bundle(unpack_bundle_path, unpack_output_dir);
  }

  if (assets_dir) {
    printf("assets root: %s\n", assets_dir);
    printf("expected bundle layout: extracted ISO tree with CGO/DGO files on disk\n");
  }

  Renderer renderer = {0};
  if (!renderer_init(&renderer, width, height, "jakc")) {
    return 1;
  }

  while (!renderer_should_close(&renderer)) {
    float now = (float)glfwGetTime();
    renderer_poll_events();
    renderer_begin_frame(&renderer, now);
    renderer_end_frame(&renderer);
  }

  renderer_shutdown(&renderer);
  return 0;
}
