#include <stdio.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

#define ASSERT(x, ...) if (!(x)) { printf(__VA_ARGS__); exit(1); }
#define UNI(shd, uni) (glGetUniformLocation(shd, uni))

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;

typedef struct {
  u16 width, height;
  f32 fov, near, far, pitch, yaw;
  vec3 pos, dir, rig;
} Camera;

typedef struct {
  i16 width, height;
  GLFWwindow* window;
} Canvas;

typedef struct {
  GLenum wrap_s, wrap_t, min_filter, mag_filter;
  u8 capture_mouse, use_keyboard, use_mouse;
  void (*key_callback)   (GLFWwindow*, i32, i32, i32, i32);
  void (*mouse_callback) (GLFWwindow*, f64, f64);
  char* title;
} CanvasInitConfig;

// --- Function

void canvas_init(Canvas* canvas, CanvasInitConfig config) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  canvas->window = glfwCreateWindow(canvas->width, canvas->height, config.title, NULL, NULL);
  ASSERT(canvas->window, "Failed creating a window");
  glfwMakeContextCurrent(canvas->window);
  ASSERT(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress), "Failed loading glad");
  glViewport(0, 0, canvas->width, canvas->height);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0, 0, 0, 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     config.wrap_s);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     config.wrap_t);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.mag_filter);
  if (config.capture_mouse) glfwSetInputMode(canvas->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (config.use_keyboard)  glfwSetKeyCallback(canvas->window, config.key_callback);
  if (config.use_mouse)     glfwSetCursorPosCallback(canvas->window, config.mouse_callback);
}

// --- Matrix

void generate_proj_mat(Camera cam, mat4 to) {
  glm_mat4_identity(to);
  glm_perspective(cam.fov, (f32) cam.width / cam.height, cam.near, cam.far, to);
}

void generate_view_mat(Camera cam, mat4 to) {
  vec3 target, up;
  glm_cross(cam.rig, cam.dir, up);
  glm_vec3_add(cam.pos, cam.dir, target);
  glm_lookat(cam.pos, target, up, to);
}

// --- Texture

u32 canvas_create_texture(GLenum unit, char path[]) {
  FILE* img = fopen(path, "r");
  ASSERT(img != NULL, "Can't open image");
  u16 width, height;
  fscanf(img, "%*s %hi %hi %*i", &width, &height);
  f32* buffer = malloc(sizeof(f32) * width * height * 3);

  for (u32 i = 0; i < width * height * 3; i += 3) {
    fscanf(img, "%f %f %f", &buffer[i], &buffer[i + 1], &buffer[i + 2]);
    buffer[i]     /= 255;
    buffer[i + 1] /= 255;
    buffer[i + 2] /= 255;
  }
  fclose(img); 

  u32 texture;
  glGenTextures(1, &texture);
  glActiveTexture(unit);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, buffer);
  glGenerateMipmap(GL_TEXTURE_2D);

  free(buffer);
  return texture;
}

// --- Object

u32 canvas_create_VBO(u32 size, const void* data, GLenum usage) {
  u32 VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, size, data, usage);
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

void canvas_vertex_attrib_pointer(u8 location, u8 amount, GLenum type, GLenum normalize, u16 stride, void* offset) {
  glVertexAttribPointer(location, amount, type, normalize, stride, offset);
  glEnableVertexAttribArray(location);
}

// --- Shader

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

void canvas_uni1i(u16 s, char u[], i32 v1)                 { glUniform1i(UNI(s, u), v1); }
void canvas_uni1f(u16 s, char u[], f32 v1)                 { glUniform1f(UNI(s, u), v1); }
void canvas_uni2i(u16 s, char u[], i32 v1, i32 v2)         { glUniform2i(UNI(s, u), v1, v2); }
void canvas_uni2f(u16 s, char u[], f32 v1, f32 v2)         { glUniform2f(UNI(s, u), v1, v2); }
void canvas_uni3i(u16 s, char u[], i32 v1, i32 v2, i32 v3) { glUniform3i(UNI(s, u), v1, v2, v3); }
void canvas_uni3f(u16 s, char u[], f32 v1, f32 v2, f32 v3) { glUniform3f(UNI(s, u), v1, v2, v3); }
void canvas_unim4(u16 s, char u[], const f32* v)           { glUniformMatrix4fv(UNI(s, u), 1, GL_FALSE, (const f32*) (v)); }