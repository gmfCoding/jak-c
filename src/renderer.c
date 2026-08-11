#include "jakc/renderer.h"

#include <math.h>
#include <stdio.h>

#include <glad/glad.h>

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  (void)window;
  glViewport(0, 0, width, height);
}

bool renderer_init(Renderer* renderer, int width, int height, const char* title) {
  if (!renderer) {
    return false;
  }

  renderer->window = NULL;
  renderer->width = width;
  renderer->height = height;

  if (!glfwInit()) {
    fprintf(stderr, "failed to initialize GLFW\n");
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

  renderer->window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!renderer->window) {
    fprintf(stderr, "failed to create GLFW window\n");
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(renderer->window);
  glfwSwapInterval(1);
  glfwSetFramebufferSizeCallback(renderer->window, framebuffer_size_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "failed to initialize GLAD\n");
    glfwDestroyWindow(renderer->window);
    renderer->window = NULL;
    glfwTerminate();
    return false;
  }

  glViewport(0, 0, width, height);
  return true;
}

void renderer_shutdown(Renderer* renderer) {
  if (!renderer) {
    return;
  }

  if (renderer->window) {
    glfwDestroyWindow(renderer->window);
    renderer->window = NULL;
  }
  glfwTerminate();
}

bool renderer_should_close(const Renderer* renderer) {
  return !renderer || !renderer->window || glfwWindowShouldClose(renderer->window);
}

void renderer_poll_events(void) {
  glfwPollEvents();
}

void renderer_begin_frame(Renderer* renderer, float time_seconds) {
  float r = 0.08f + 0.08f * (0.5f + 0.5f * sinf(time_seconds * 0.9f));
  float g = 0.10f + 0.10f * (0.5f + 0.5f * sinf(time_seconds * 0.7f + 2.0f));
  float b = 0.14f + 0.14f * (0.5f + 0.5f * sinf(time_seconds * 0.5f + 4.0f));
  (void)renderer;
  glClearColor(r, g, b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_end_frame(Renderer* renderer) {
  glfwSwapBuffers(renderer->window);
}
