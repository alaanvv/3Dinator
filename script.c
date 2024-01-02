#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "shader.h"
#include "misc.h"

#define PI 3.14159
#define TAU PI * 2
#define ASSERT(x, str) if (!(x)) {printf(str); exit(1);}
#define WIDTH  800
#define HEIGHT 800
#define UNI(shader, name) (glGetUniformLocation(shader, name))

typedef unsigned int uint;
typedef uint8_t u8;
typedef uint16_t u16;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef float f32;
typedef double f64;

typedef struct {
  GLFWwindow* window;

  i16 width;
  i16 height;
} Canvas;

u8 wireframe_mode;
Canvas canvas = { NULL, WIDTH, HEIGHT };

f32 vertices[] = {
  0, 0.75, 0,  0.5, 0,
  -0.75, -0.5, 0,  0,   1,
  0.75, -0.5, 0,  1,   1
};

// ---

void canvas_init(Canvas* canvas);
void handle_inputs(GLFWwindow* window, int key, int scancode, int action, int mods);
uint canvas_create_VBO();
uint canvas_create_VAO();
uint canvas_create_EBO();
uint canvas_create_EBO();
uint canvas_create_texture(GLenum unit, char path[]);

i8 main() {
  canvas_init(&canvas);
  glfwSetKeyCallback(canvas.window, handle_inputs);

  uint VBO = canvas_create_VBO();
  uint VAO = canvas_create_VAO();

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*) 0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*) (sizeof(f32) * 3));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  uint shader_program = shader_create_program("shd/vertex.glsl", "shd/fragment.glsl");

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  uint texture0_ID = canvas_create_texture(GL_TEXTURE0, "img/bills.ppm");
  glUniform1i(glGetUniformLocation(shader_program, "sampler0"), 0);
  uint texture1_ID = canvas_create_texture(GL_TEXTURE1, "img/ilu.ppm");
  glUniform1i(glGetUniformLocation(shader_program, "sampler1"), 1);

  // Transform
  vec4 vec = { 1, 0, 0, 1 };
  mat4 trans;
  glm_mat4_identity(trans);

  glm_translate(trans, (vec3) { 1, 1, 0 });
  glm_mat4_mulv(trans, vec, vec);


  while (!glfwWindowShouldClose(canvas.window)) {
    glUniform1f(glGetUniformLocation(shader_program, "time"), glfwGetTime());
    glClearColor(0.35, 0.35, 0.35, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(canvas.window); 
    glfwPollEvents();    
  }

  glfwTerminate();
  return 0;
}

void canvas_init(Canvas* canvas) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  canvas->window = glfwCreateWindow(canvas->width, canvas->height, "", NULL, NULL);
  ASSERT(canvas->window, "Failed to create a window");
  glfwMakeContextCurrent(canvas->window);

  ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed loading glad");
  glViewport(0, 0, WIDTH, HEIGHT);
}

uint canvas_create_VBO() {
  uint VBO_ID;
  glGenBuffers(1, &VBO_ID);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);

  return VBO_ID;
}

uint canvas_create_VAO() {
  uint VAO_ID;
  glGenVertexArrays(1, &VAO_ID);
  glBindVertexArray(VAO_ID);

  return VAO_ID;
}

uint canvas_create_EBO() {
  uint EBO_ID;
  glGenBuffers(1, &EBO_ID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ID);

  return EBO_ID;
}

uint canvas_create_texture(GLenum unit, char path[]) {
  uint texture_ID;
  glGenTextures(1, &texture_ID);
  glActiveTexture(unit);
  glBindTexture(GL_TEXTURE_2D, texture_ID);

  u16 width, height;
  get_image_size(path, &width, &height);
  f32* image_buffer = malloc(sizeof(f32) * width * height * 3);
  load_image(path, image_buffer, width * height);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, image_buffer);
  glGenerateMipmap(GL_TEXTURE_2D);

  free(image_buffer);
  return texture_ID;
}

void handle_inputs(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(canvas.window, 1); 

  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    if (wireframe_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    wireframe_mode = 1 - wireframe_mode;
  }
}
