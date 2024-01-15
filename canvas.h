#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>

#define PI 3.14159
#define PI2 PI / 2
#define ASSERT(x, ...) if (!(x)) { printf(__VA_ARGS__); exit(1); }

typedef uint16_t u16;
typedef uint32_t u32;
typedef int16_t  i16;
typedef int32_t  i32;
typedef float    f32;

// --- Type

typedef struct { 
  f32 R, G, B, A; 
} RGBA;

typedef struct {
  i16 width, height;
  f32 fov, near, far;

  vec3 pos, dir, rig;
  f32  pitch, yaw;
} Camera;

typedef struct {
  GLFWwindow* window;

  i16 width, height;
} Canvas;

// --- Function

void generate_proj_mat(Camera cam, mat4 to) {
  glm_mat4_identity(to);
  glm_perspective(cam.fov, (f32) cam.width / cam.height, cam.near, cam.far, to);
}

void generate_view_mat(Camera cam, mat4 to) {
  vec3 target, up;
  glm_cross(cam.dir, cam.rig, up);
  up[1] *= -1;

  glm_vec3_add(cam.pos, cam.dir, target);
  glm_lookat(cam.pos, target, up, to);
}

void canvas_init(Canvas* canvas, char title[]) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  canvas->window = glfwCreateWindow(canvas->width, canvas->height, title, NULL, NULL);
  ASSERT(canvas->window, "Failed creating a window");
  glfwMakeContextCurrent(canvas->window);

  ASSERT(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress), "Failed loading glad");
  glViewport(0, 0, canvas->width, canvas->height);
  glEnable(GL_DEPTH_TEST);
}

void canvas_clear(RGBA color) {
  glClearColor(color.R, color.G, color.B, color.A);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
  glUniform1i(glGetUniformLocation(shader, uniform_name), uniform_value);

  free(image_buffer);
  return texture_ID;
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

  u32 v_shader = create_shader(GL_VERTEX_SHADER, vertex_path, "vertex");
  u32 f_shader = create_shader(GL_FRAGMENT_SHADER, fragment_path, "fragment");
  u32 shader_program = glCreateProgram();
  glAttachShader(shader_program, v_shader);
  glAttachShader(shader_program, f_shader);
  glLinkProgram(shader_program);
  glDeleteShader(v_shader);
  glDeleteShader(f_shader);

  i32 success;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  ASSERT(success, "Error linking shaders");

  glUseProgram(shader_program);
  return shader_program;
}

u32 shader_create_program_raw(const char* v_shader_source, const char* f_shader_source) {
  u32 v_shader = glCreateShader(GL_VERTEX_SHADER);
  u32 f_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(v_shader, 1, &v_shader_source, NULL);
  glShaderSource(f_shader, 1, &f_shader_source, NULL);
  glCompileShader(v_shader);
  glCompileShader(f_shader);

  u32 shader_program = glCreateProgram();
  glAttachShader(shader_program, v_shader);
  glAttachShader(shader_program, f_shader);
  glLinkProgram(shader_program);
  glDeleteShader(v_shader);
  glDeleteShader(f_shader);

  glUseProgram(shader_program);
  return shader_program;
}
