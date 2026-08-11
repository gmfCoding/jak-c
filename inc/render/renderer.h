#pragma once

#include <stdbool.h>

#include <GLFW/glfw3.h>

typedef struct Renderer {
  GLFWwindow* window;
  int width;
  int height;
} Renderer;

bool renderer_init(Renderer* renderer, int width, int height, const char* title);
void renderer_shutdown(Renderer* renderer);
bool renderer_should_close(const Renderer* renderer);
void renderer_poll_events(void);
void renderer_begin_frame(Renderer* renderer, float time_seconds);
void renderer_end_frame(Renderer* renderer);
