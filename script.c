#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>

#define ASSERT(x, ...) if (!(x)) { printf(__VA_ARGS__); exit(1); }
#define UNI(shader, name) (glGetUniformLocation(shader, name))
#define PI  3.14159
#define PI2 PI / 2
#define PI4 PI / 4
#define TAU PI * 2

#define VERTEX_SHADER   "shd/vertex.glsl"
#define FRAGMENT_SHADER "shd/fragment.glsl"
#define TEXTURE_0 "img/pmk.ppm"
#define TEXTURE_1 "img/ilu.ppm"
#define WIDTH  800
#define HEIGHT 800
#define MOVEMENT_SPEED 0.01

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;

// --- Type

typedef struct { f32 R, G, B, A; } RGBA;

typedef struct {
  i16 width, height;
  f32 fov, near, far;

  f32 pos[3];
  f32 ang[2];
} Camera;

typedef struct {
  GLFWwindow* window;

  i16 width, height;
} Canvas;

u32 canvas_create_texture(GLenum unit, char path[], u32 shader, char uniform_name[], i32 uniform_value);
void canvas_config_texture(GLenum wrap_s, GLenum wrap_t, GLenum min_filter, GLenum mag_filter);
void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
u32 shader_create_program(char vertex_path[], char fragment_path[]);
void get_image_size(char path[], u16* width, u16* height);
void load_image(char path[], f32 buffer[], u32 pixels);
void generate_proj_mat(Camera cam, mat4 to);
void generate_view_mat(Camera cam, mat4 to);
void handle_inputs(GLFWwindow* window);
void canvas_init(Canvas* canvas);
void canvas_clear(RGBA color);
u32 canvas_create_VBO();
u32 canvas_create_VAO();
u32 canvas_create_EBO();

// --- Setup

u8 wireframe_mode;

Canvas canvas = { NULL, WIDTH, HEIGHT };
Camera cam = { WIDTH, HEIGHT, PI4, 0.1, 100, { 0, 0, 0 }, { 0, 0 } };

f32 vertices[] = {
  // Face
  -0.5, -0.5, -0.5,  0, 0,
  0.5, -0.5, -0.5,  0.333, 0,
  0.5,  0.5, -0.5,  0.333, 1,
  0.5,  0.5, -0.5,  0.333, 1,
  -0.5,  0.5, -0.5,  0, 1,
  -0.5, -0.5, -0.5,  0, 0,

  // Back
  -0.5, -0.5,  0.5,  0.666, 0,
  0.5, -0.5,  0.5,  1, 0,
  0.5,  0.5,  0.5,  1, 1,
  0.5,  0.5,  0.5,  1, 1,
  -0.5,  0.5,  0.5,  0.666, 1,
  -0.5, -0.5,  0.5,  0.666, 0,

  -0.5,  0.5,  0.5,  0.666, 1,
  -0.5,  0.5, -0.5,  1, 1,
  -0.5, -0.5, -0.5,  1, 0,
  -0.5, -0.5, -0.5,  1, 0,
  -0.5, -0.5,  0.5,  0.666, 0,
  -0.5,  0.5,  0.5,  0.666, 1,

  0.5,  0.5,  0.5,  0.666, 1,
  0.5,  0.5, -0.5,  1, 1,
  0.5, -0.5, -0.5,  1, 0,
  0.5, -0.5, -0.5,  1, 0,
  0.5, -0.5,  0.5,  0.666, 0,
  0.5,  0.5,  0.5,  0.666, 1,

  // Up
  -0.5, -0.5, -0.5,  0.333, 1,
  0.5, -0.5, -0.5,  .666, 1,
  0.5, -0.5,  0.5,  .666, 0,
  0.5, -0.5,  0.5,  .666, 0,
  -0.5, -0.5,  0.5,  0.333, 0,
  -0.5, -0.5, -0.5,  0.333, 1,

  // Bottom
  -0.5,  0.5, -0.5,  0.333, 1,
  0.5,  0.5, -0.5,  .666, 1,
  0.5,  0.5,  0.5,  .666, 0,
  0.5,  0.5,  0.5,  .666, 0,
  -0.5,  0.5,  0.5,  0.333, 0,
  -0.5,  0.5, -0.5,  0.333, 1
};

mat4 view, proj;

// --- Main

i8 main() {
  canvas_init(&canvas);
  glfwSetKeyCallback(canvas.window, key_callback);

  u32 VBO = canvas_create_VBO();
  u32 VAO = canvas_create_VAO();

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*) 0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*) (sizeof(f32) * 3));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  u32 shader_program = shader_create_program(VERTEX_SHADER, FRAGMENT_SHADER);

  canvas_config_texture(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_NEAREST, GL_NEAREST);
  canvas_create_texture(GL_TEXTURE0, TEXTURE_0, shader_program, "TEXTURE_0", 0);
  canvas_create_texture(GL_TEXTURE1, TEXTURE_1, shader_program, "TEXTURE_1", 1);

  mat4 model;
  generate_proj_mat(cam, proj);
  generate_view_mat(cam, view);

  vec3 poss[] = {
    { -3/2, 10, -10 },
    { 6/2, 15, -5 },
    { -8/2, 12, -5 },
    { 3/2, 9, -12 },
    { -4/2, 10, -3 },
    { 3/2, 15, -4 },
    { -2/2, 11, -5 },
    { 3/2, 16, -3 },
    { -5/2, 7, -1 },
    { 9/2, 13, -3 },
    { -9/2, 18, -10 },
    { -7/2, 8, -8 },
    { 8/2, 14, -6 },
    { -10/2, 11, -7 },
    { 4/2, 8, -15 },
    { -5/2, 9, -2 },
    { 4/2, 14, -3 },
    { -4/2, 10, -6 },
    { 5/2, 15, -2 },
    { -6/2, 6, 0 },
    { 10/2, 12, -2 },
    { -10/2, 17, -9 },
    { -6/2, 16, -1 },
    { 7/2, 10, -10 },
    { -2/2, 15, -8 },
    { 8/2, 9, -11 },
    { -3/2, 14, -4 },
    { 6/2, 17, -7 },
    { -9/2, 13, -5 },
    { 11/2, 18, -9 },
    { -8/2, 19, -8 },
  };

  while (!glfwWindowShouldClose(canvas.window)) {
    canvas_clear((RGBA) { 0.835, 0.741, 0.686, 1 });

    for (u8 i = 0; i < 30; i++) {
      glm_mat4_identity(model);
      glm_translate(model, poss[i]);
      glm_rotate(model, glfwGetTime() * tan(i + 3), (vec3) { 0.5, 0.2, 0 });
      poss[i][1] -= 0.05;
      if (poss[i][1] / poss[i][2] > 1) poss[i][1] = abs(poss[i][2]);
      glUniformMatrix4fv(UNI(shader_program, "MODEL"), 1, GL_FALSE, (const f32*) { model[0] });
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Re-send some matrices that might have changed
    glUniformMatrix4fv(UNI(shader_program, "PROJ"),  1, GL_FALSE, (const f32*) { proj[0]  });
    glUniformMatrix4fv(UNI(shader_program, "VIEW"),  1, GL_FALSE, (const f32*) { view[0]  });


    glfwSwapBuffers(canvas.window); 
    glfwPollEvents();
    handle_inputs(canvas.window);    
  }

  glfwTerminate();
  return 0;
}

// --- Function

void generate_proj_mat(Camera cam, mat4 to) {
  glm_mat4_identity(to);
  glm_perspective(cam.fov, (f32) cam.width / cam.height, cam.near, cam.far, to);
}

void generate_view_mat(Camera cam, mat4 to) {
  glm_mat4_identity(to);
  glm_translate(view, (vec3) { -cam.pos[0], -cam.pos[1], -cam.pos[2] });
}

void canvas_init(Canvas* canvas) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  canvas->window = glfwCreateWindow(canvas->width, canvas->height, "Coordinate System", NULL, NULL);
  ASSERT(canvas->window, "Failed creating a window");
  glfwMakeContextCurrent(canvas->window);

  ASSERT(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress), "Failed loading glad");
  glViewport(0, 0, WIDTH, HEIGHT);
  glEnable(GL_DEPTH_TEST);
}

void canvas_clear(RGBA color) {
  glClearColor(color.R, color.G, color.B, color.A);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void canvas_config_texture(GLenum wrap_s, GLenum wrap_t, GLenum min_filter, GLenum mag_filter) {
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}

u32 canvas_create_texture(GLenum unit, char path[], u32 shader, char uniform_name[], i32 uniform_value) {
  u32 texture_ID;
  glGenTextures(1, &texture_ID);
  glActiveTexture(unit);
  glBindTexture(GL_TEXTURE_2D, texture_ID);

  u16 width, height;
  get_image_size(path, &width, &height);
  f32* image_buffer = malloc(sizeof(f32) * width * height * 3);
  load_image(path, image_buffer, width * height);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, image_buffer);
  glGenerateMipmap(GL_TEXTURE_2D);
  glUniform1i(UNI(shader, uniform_name), uniform_value);

  free(image_buffer);
  return texture_ID;
}

void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
  if (action != GLFW_PRESS) return;

  switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(canvas.window, 1); 
      break;

    case GLFW_KEY_M:
      if (wireframe_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      wireframe_mode = 1 - wireframe_mode;
  }
}

void handle_inputs(GLFWwindow* window) {
  u8 should_reload_view, should_reload_proj;

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
  { cam.pos[2] += MOVEMENT_SPEED; should_reload_view = 1; }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  { cam.pos[2] -= MOVEMENT_SPEED; should_reload_view = 1; }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  { cam.pos[0] -= MOVEMENT_SPEED; should_reload_view = 1; }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  { cam.pos[0] += MOVEMENT_SPEED; should_reload_view = 1; }
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
  { cam.pos[1] += MOVEMENT_SPEED; should_reload_view = 1; }
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
  { cam.pos[1] -= MOVEMENT_SPEED; should_reload_view = 1; }

  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
  { cam.fov += PI / 100; should_reload_proj = 1; }
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
  { cam.fov -= PI / 100; should_reload_proj = 1; }

  if (should_reload_view) generate_view_mat(cam, view);
  if (should_reload_proj) generate_proj_mat(cam, proj);
}

u32 canvas_create_VBO() {
  u32 VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  return VBO;
}

u32 canvas_create_VAO() {
  u32 VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  return VAO;
}

u32 canvas_create_EBO() {
  u32 EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  return EBO;
}

void get_image_size(char path[], u16* width, u16* height) {
  FILE* img = fopen(path, "r");
  ASSERT(img != NULL, "Can't open image");

  fscanf(img, "%*s %hi %hi", width, height);

  fclose(img);
}

void load_image(char path[], f32 buffer[], u32 pixels) {
  FILE* img = fopen(path, "r");
  ASSERT(img != NULL, "Can't open image");
  fscanf(img, "%*s %*i %*i %*s");

  int pixel[3];
  for (u32 i = 0; i < pixels; i++) {
    fscanf(img, "%i %i %i", &pixel[0], &pixel[1], &pixel[2]);
    buffer[i * 3 + 0] = (f32) pixel[0] / 255;
    buffer[i * 3 + 1] = (f32) pixel[1] / 255;
    buffer[i * 3 + 2] = (f32) pixel[2] / 255;
  }

  fclose(img);
}

u32 shader_create_program(char vertex_path[], char fragment_path[]) {
  u32 create_shader(GLenum type, char path[], char name[]) {
    FILE* file = fopen(path, "r");
    ASSERT(file != NULL, "Can't open %s shader", name);
    i32 success;

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char shader_source[size];
    fread(shader_source, sizeof(char), size - 1, file);
    shader_source[size - 1] = '\0';
    fclose(file);

    u32 shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char * const *) &(const char *) { shader_source }, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    ASSERT(success, "Error compiling %s shader", name);
    return shader;
  }

  u32 vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_path, "vertex");
  u32 fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_path, "fragment");
  u32 shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  i32 success;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  ASSERT(success, "Error linking shaders");

  glUseProgram(shader_program);
  return shader_program;
}
