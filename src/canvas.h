#include <dirent.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <GLFW/glfw3.h>
#include <string.h>

#define MAX_SOUNDS   100
#define MAX_TEXTURES 100
#define MAX_ENTITIES 100

#define UNI(shd, uni) (glGetUniformLocation(shd, uni))

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, y, z) (MAX(MIN(z, y), x))
#define CIRCULAR_CLAMP(x, y, z) ((y < x) ? z : ((y > z) ? x : y))
#define RAND(min, max) (rand() % (max - min) + min)
#define PRINT(...) { printf(__VA_ARGS__); printf("\n"); }
#define ASSERT(x, ...) if (!(x)) { PRINT(__VA_ARGS__); exit(1); }
#define EXIT(...) { PRINT(__VA_ARGS__); exit(1); }
#define VEC2_COPY(v1, v2) { v2[0] = v1[0]; v2[1] = v1[1]; }
#define VEC2_COMPARE(v1, v2) (v1[0] == v2[0] && v1[1] == v2[1])
#define VEC3_COPY(v1, v2) { v2[0] = v1[0]; v2[1] = v1[1]; v2[2] = v1[2]; }
#define VEC3_ADD(v1, v2) { v1[0] += v2[0]; v1[1] += v2[1]; v1[2] += v2[2]; }
#define VEC3_COMPARE(v1, v2) (v1[0] == v2[0] && v1[1] == v2[1] && v1[2] == v2[2])
#define VERTEX_COPY(from, to) { for (u8 i_ = 0; i_ < 8; i_++) to[i_] = from[i_]; }
#define VEC2(a, b)    (vec2) { a, b }
#define VEC3(a, b, c) (vec3) { a, b, c }
#define LEN(v) ( sizeof(v) / sizeof(v[0]) )

#define PI  3.14159
#define TAU PI * 2
#define PI2 PI / 2
#define PI4 PI / 4

#define WHITE         { 1.00, 1.00, 1.00 }
#define GRAY          { 0.50, 0.50, 0.50 }
#define BLACK         { 0.00, 0.00, 0.00 }
#define PURPLE        { 0.55, 0.41, 0.62 }
#define PASTEL_BLUE   { 0.69, 0.87, 1.00 }
#define PASTEL_PINK   { 1.00, 0.75, 0.79 }
#define PASTEL_GREEN  { 0.60, 0.98, 0.60 }
#define PASTEL_YELLOW { 1.00, 1.00, 0.60 }
#define YELLOW        { 0.80, 0.80, 0.30 }
#define PASTEL_PURPLE { 0.80, 0.70, 1.00 }
#define DEEP_RED      { 0.60, 0.00, 0.00 }
#define DEEP_BLUE     { 0.00, 0.00, 0.50 }
#define DEEP_GREEN    { 0.00, 0.50, 0.00 }
#define DEEP_PURPLE   { 0.40, 0.00, 0.60 }
#define DEEP_ORANGE   { 1.00, 0.27, 0.00 }

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;
typedef char     c8;

void texture_load_all();
void init_audio_engine();
u32 canvas_create_VAO();
u32 canvas_create_VBO(u32, const void*, GLenum);
void canvas_vertex_attrib_pointer(u8, u8, GLenum, GLenum, u16, void*);

// Canvas

typedef struct {
  f32 fov, near_plane, far_plane, sensitivity, camera_lock, speed, pitch, yaw, fps;
  vec3 pos, dir, rig;
  u16 width, height;
  GLFWwindow* window;
  mat4 view, proj, ortho;
} Camera;

typedef struct {
  char* title;
  u8 capture_mouse, fullscreen;
  f32 screen_size;
  vec3 clear_color;
} CanvasConfig;

u32 PLANE_VAO, PLANE_VBO;

void canvas_init(Camera* cam, CanvasConfig config) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  cam->width  = mode->width  * config.screen_size;
  cam->height = mode->height * config.screen_size;
  cam->window = glfwCreateWindow(cam->width, cam->height, config.title, config.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
  glfwMakeContextCurrent(cam->window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);

  glClearColor(config.clear_color[0], config.clear_color[1], config.clear_color[2], 1);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (config.capture_mouse) glfwSetInputMode(cam->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glm_vec3_copy(VEC3(0, 0, -1), cam->dir);
  glm_vec3_copy(VEC3(1, 0,  0), cam->rig);

  u32 tex_w, tex_b;
  glGenTextures(1, &tex_w);
  glActiveTexture(GL_TEXTURE29);
  glBindTexture(GL_TEXTURE_2D, tex_w);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, (f32[]) WHITE);

  glGenTextures(1, &tex_b);
  glActiveTexture(GL_TEXTURE30);
  glBindTexture(GL_TEXTURE_2D, tex_b);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, (f32[]) BLACK);

  f32 square[6][5] =  { 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 };
  PLANE_VAO = canvas_create_VAO();
  PLANE_VBO = canvas_create_VBO(30 * sizeof(f32), square, GL_STATIC_DRAW);
  canvas_vertex_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*) 0);
  canvas_vertex_attrib_pointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*) (3 * sizeof(f32)));

  init_audio_engine();
  texture_load_all();
}

void generate_proj_mat(Camera* cam, u32 shader) {
  glm_mat4_identity(cam->proj);
  glm_perspective(cam->fov, (f32) cam->width / cam->height, cam->near_plane, cam->far_plane, cam->proj);
  glUniformMatrix4fv(UNI(shader, "PROJ"), 1, GL_FALSE, cam->proj[0]);
}

void generate_view_mat(Camera* cam, u32 shader) {
  vec3 target, up;
  glm_cross(cam->rig, cam->dir, up);
  glm_vec3_add(cam->pos, cam->dir, target);
  glm_lookat(cam->pos, target, up, cam->view);
  glUniformMatrix4fv(UNI(shader, "VIEW"), 1, GL_FALSE, cam->view[0]);
}

void generate_ortho_mat(Camera* cam, u32 shader) {
  glm_ortho(0, cam->width, 0, cam->height, -1.0, 1.0, cam->ortho);
  glUniformMatrix4fv(UNI(shader, "PROJ"), 1, GL_FALSE, cam->ortho[0]);
}

void update_fps(Camera* cam) {
  static f32 tick = 0;
  cam->fps = 1 / (glfwGetTime() - tick);
  tick = glfwGetTime();
}

// Object

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

u32 canvas_create_FBO(u16 width, u16 height, GLenum min, GLenum mag) {
  u32 FBO;
  glGenFramebuffers(1, &FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  u32 REN_TEX;
  glGenTextures(1, &REN_TEX);
  glActiveTexture(GL_TEXTURE31);
  glBindTexture(GL_TEXTURE_2D, REN_TEX);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, min);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mag);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, REN_TEX, 0);

  return FBO;
}

void canvas_vertex_attrib_pointer(u8 location, u8 amount, GLenum type, GLenum normalize, u16 stride, void* offset) {
  glVertexAttribPointer(location, amount, type, normalize, stride, offset);
  glEnableVertexAttribArray(location);
}

// Shader

u32 _create_shader(GLenum type, char path[]) {
  FILE* file = fopen(path, "r");
  ASSERT(file, "Can't open shader (%s)", path);
  i32 success;

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char shader_source[size];
  fread(shader_source, sizeof(char), size - 1, file);
  shader_source[size - 1] = '\0';
  fclose(file);

  u32 shader = glCreateShader(type);
  const char* _shader_source = shader_source;
  glShaderSource(shader, 1, &_shader_source, NULL);
  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  ASSERT(success, "Error compiling shader (%s)", path);
  return shader;
}

u32 shader_create_program(char name[]) {
  c8 vertex_path[64] = { 0 };
  sprintf(vertex_path, "shd/%s.v", name);
  u32 v_shader = _create_shader(GL_VERTEX_SHADER, vertex_path);

  c8 fragment_path[64] = { 0 };
  sprintf(fragment_path, "shd/%s.f", name);
  u32 f_shader = _create_shader(GL_FRAGMENT_SHADER, fragment_path);

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
void canvas_unim4(u16 s, char u[], const f32* m)           { glUniformMatrix4fv(UNI(s, u), 1, GL_FALSE, m); }

// Texture

#define TEXTURE_DEFAULT (TextureConfig) { GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_NEAREST, GL_NEAREST }

typedef struct {
  GLenum wrap_s, wrap_t, min_filter, mag_filter;
} TextureConfig;

typedef struct {
  GLenum texture_unit;
  c8 name[32];
} TextureEntity;

TextureEntity textures[MAX_TEXTURES];
u8 textures_size = 0;

u32 canvas_create_texture(GLenum unit, char* name, TextureConfig config) {
  c8 path[64] = { 0 };
  sprintf(path, "img/%s", name);

  FILE* img = fopen(path, "r");
  ASSERT(img, "Can't open image (%s)", path);

  u16 ppm, width, height, maxval;
  fscanf(img, "P%hi %hi %hi %hi", &ppm, &width, &height, &maxval);

  f32* buffer = malloc(sizeof(f32) * width * height * 3);
  f32 _temp[3];
  u8   temp[3];

  if (ppm == 3) {
    for (u32 i = 0; i < width * height * 3; i += 3) {
      fscanf(img, "%f %f %f", &buffer[i], &buffer[i + 1], &buffer[i + 2]);
      glm_vec3_scale(&buffer[i], (f32) 1 / maxval, &buffer[i]);
    }
  }

  if (ppm == 6) {
    img = fopen(path, "rb");
    fscanf(img, "P%*d %*d %*d %*d\n");
    for (u32 i = 0; i < width * height * 3; i += 3) {
      fread(temp, 1 + (maxval > 255), 3, img);
      VEC3_COPY(temp, _temp);
      glm_vec3_scale(_temp, (f32) 1 / maxval, &buffer[i]);
    }
  }

  fclose(img);

  u32 texture;
  glGenTextures(1, &texture);
  glActiveTexture(unit);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     config.wrap_s);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     config.wrap_t);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.mag_filter);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, buffer);
  glGenerateMipmap(GL_TEXTURE_2D);

  free(buffer);
  return texture;
}

void texture_load_all() {
  struct dirent* de;
  DIR* dir = opendir("./img");
  ASSERT(dir, "Could not open \"img\" directory");

  while ((de = readdir(dir)) != NULL) {
    if (de->d_type != DT_REG) continue;
    c8 buffer[64] = { 0 };
    sprintf(buffer, "img/%s", de->d_name);

    GLenum unit = GL_TEXTURE0 + textures_size;
    canvas_create_texture(unit, de->d_name, TEXTURE_DEFAULT);
    textures[textures_size].texture_unit = unit;
    strcpy(textures[textures_size++].name, de->d_name);
  }
}

GLenum texture(c8* name) {
  c8 buffer[64] = { 0 };
  sprintf(buffer, "%s.ppm", name);
  for (int i = 0; i < textures_size; i++) {
    if (strcmp(textures[i].name, buffer)) continue;
    return textures[i].texture_unit;
  }

  EXIT("Texture \"%s\" not found", name);
}

// Material

typedef struct {
  vec3 col;
  f64  amb, dif;
  GLenum tex, emt;
  u8 lig;
} Material;

void canvas_set_material(u32 shader, Material mat) {
  canvas_uni3f(shader, "MAT.COL", mat.col[0], mat.col[1], mat.col[2]);
  canvas_uni1f(shader, "MAT.AMB", mat.amb);
  canvas_uni1f(shader, "MAT.DIF", mat.dif);
  canvas_uni1i(shader, "MAT.S_DIF", mat.tex >= GL_TEXTURE0 ? (mat.tex - GL_TEXTURE0) : 29);
  canvas_uni1i(shader, "MAT.S_EMT", mat.emt >= GL_TEXTURE0 ? (mat.emt - GL_TEXTURE0) : 30);
  canvas_uni1i(shader, "MAT.LIG", mat.lig);
}

// Model

typedef f32 Vertex[8];

typedef struct {
  u32 size, VAO, VBO;
  Vertex* vertexes;
  mat4 model;
  Material material;
} Model;

void model_parse(Model* model, const c8* path, u32* size, f32 scale) {
  vec3*   poss = malloc(sizeof(vec3));
  vec2*   texs = malloc(sizeof(vec2));
  Vertex* square = malloc(sizeof(Vertex));

  u32 pos_i = 0;
  u32 tex_i = 0;
  u32 vrt_i = 0;

  FILE* file = fopen(path, "r");
  c8 buffer[256];
  while (fgets(buffer, 256, file)) {
    if      (buffer[0] == 'v' && buffer[1] == ' ') {
      poss = realloc(poss, sizeof(vec3) * (++pos_i + 1));
      sscanf(buffer, "v %f %f %f", &poss[pos_i][0], &poss[pos_i][1], &poss[pos_i][2]);
      glm_vec3_scale(poss[pos_i], scale, poss[pos_i]);
    }
    else if (buffer[0] == 'v' && buffer[1] == 't') {
      texs = realloc(texs, sizeof(vec2) * (++tex_i + 3));
      sscanf(buffer, "vt %f %f",    &texs[tex_i][0], &texs[tex_i][1]);
    }
    else if (buffer[0] == 'f') {
      char v_buf[4][256];
      u8 is_quad = 4 == sscanf(buffer, "f %s %s %s %s", v_buf[0], v_buf[1], v_buf[2], v_buf[3]);

      square = realloc(square, sizeof(Vertex) * (vrt_i + (is_quad ? 6 : 3)));

      for (u8 i = 0; i < 3 + is_quad; i++) {
        u32 pi, ti;
        sscanf(v_buf[i], "%d/%d", &pi, &ti);
        glm_vec3_copy(poss[pi], &square[vrt_i + i][0]);
        glm_vec3_copy(texs[ti], &square[vrt_i + i][6]);
      }

      vec3 side_1, side_2, nrm;
      glm_vec3_sub(square[vrt_i + 1], square[vrt_i + 0], side_1);
      glm_vec3_sub(square[vrt_i + 2], square[vrt_i + 0], side_2);
      glm_vec3_cross(side_1, side_2, nrm);
      glm_vec3_normalize(nrm);

      for (u8 i = 0; i < 3 + is_quad; i++)
        glm_vec3_copy(nrm, &square[vrt_i + i][3]);

      vrt_i += 3 + is_quad;
      if (!is_quad) continue;

      for (u32 i = vrt_i; vrt_i < i + 2; vrt_i++)
        VERTEX_COPY(square[vrt_i - 4 + (vrt_i - i)], square[vrt_i]);
    }
  }

  fclose(file);
  free(poss);
  free(texs);

  *size = vrt_i;
  model->vertexes = square;
}

Model* model_create(const c8* name, Material material, f32 scale) {
  c8 buffer[64] = { 0 };
  sprintf(buffer, "obj/%s.obj", name);

  Model* model = malloc(sizeof(Model));
  model_parse(model, buffer, &model->size, scale);
  model->material = material;

  model->VAO = canvas_create_VAO();
  model->VBO = canvas_create_VBO(model->size * sizeof(Vertex), model->vertexes, GL_STATIC_DRAW);
  canvas_vertex_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) 0);
  canvas_vertex_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (3 * sizeof(f32)));
  canvas_vertex_attrib_pointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (6 * sizeof(f32)));

  return model;
}

void model_bind(Model* model, u32 shader) {
  canvas_set_material(shader, model->material);
  glm_mat4_identity(model->model);
}

void model_draw(Model* model, u32 shader) {
  glBindBuffer(GL_ARRAY_BUFFER, model->VBO);
  glBindVertexArray(model->VAO);
  canvas_unim4(shader, "MODEL", model->model[0]);
  glDrawArrays(GL_TRIANGLES, 0, model->size);
}

// Light

typedef struct {
  vec3 col, dir;
} DirLig;

typedef struct {
  vec3 col, pos;
  f32  con, lin, qua;
} PntLig;

typedef struct {
  vec3 col, pos, dir;
  f32  con, lin, qua, inn, out;
} SptLig;

void canvas_set_dir_lig(u32 shader, DirLig dir_lig, u32 i) {
  canvas_uni1i(shader, "DIR_LIG_AMOUNT", i + 1);
  char uniform[255];
  sprintf(uniform, "DIR_LIGS[%i].COL", i);
  canvas_uni3f(shader, uniform, dir_lig.col[0], dir_lig.col[1], dir_lig.col[2]);
  sprintf(uniform, "DIR_LIGS[%i].DIR", i);
  canvas_uni3f(shader, uniform, dir_lig.dir[0], dir_lig.dir[1], dir_lig.dir[2]);
}

void canvas_set_pnt_lig(u32 shader, PntLig pnt_lig, u32 i) {
  canvas_uni1i(shader, "PNT_LIG_AMOUNT", i + 1);
  char uniform[255];
  sprintf(uniform, "PNT_LIGS[%i].COL", i);
  canvas_uni3f(shader, uniform, pnt_lig.col[0], pnt_lig.col[1], pnt_lig.col[2]);
  sprintf(uniform, "PNT_LIGS[%i].POS", i);
  canvas_uni3f(shader, uniform, pnt_lig.pos[0], pnt_lig.pos[1], pnt_lig.pos[2]);
  sprintf(uniform, "PNT_LIGS[%i].CON", i);
  canvas_uni1f(shader, uniform, pnt_lig.con);
  sprintf(uniform, "PNT_LIGS[%i].LIN", i);
  canvas_uni1f(shader, uniform, pnt_lig.lin);
  sprintf(uniform, "PNT_LIGS[%i].QUA", i);
  canvas_uni1f(shader, uniform, pnt_lig.qua);
}

void canvas_set_spt_lig(u32 shader, SptLig spt_lig, u32 i) {
  canvas_uni1i(shader, "SPT_LIG_AMOUNT", i + 1);
  char uniform[255];
  sprintf(uniform, "SPT_LIGS[%i].COL", i);
  canvas_uni3f(shader, uniform, spt_lig.col[0], spt_lig.col[1], spt_lig.col[2]);
  sprintf(uniform, "SPT_LIGS[%i].POS", i);
  canvas_uni3f(shader, uniform, spt_lig.pos[0], spt_lig.pos[1], spt_lig.pos[2]);
  sprintf(uniform, "SPT_LIGS[%i].DIR", i);
  canvas_uni3f(shader, uniform, spt_lig.dir[0], spt_lig.dir[1], spt_lig.dir[2]);
  sprintf(uniform, "SPT_LIGS[%i].CON", i);
  canvas_uni1f(shader, uniform, spt_lig.con);
  sprintf(uniform, "SPT_LIGS[%i].LIN", i);
  canvas_uni1f(shader, uniform, spt_lig.lin);
  sprintf(uniform, "SPT_LIGS[%i].QUA", i);
  canvas_uni1f(shader, uniform, spt_lig.qua);
  sprintf(uniform, "SPT_LIGS[%i].INN", i);
  canvas_uni1f(shader, uniform, spt_lig.inn);
  sprintf(uniform, "SPT_LIGS[%i].OUT", i);
  canvas_uni1f(shader, uniform, spt_lig.out);
}

// HUD

void hud_draw_rec(u32 shader, GLenum texture, vec3 color, i32 x, i32 y, i32 width, i32 height) {
  mat4 model;
  glm_mat4_identity(model);
  glm_translate(model, VEC3(x, y, 0));
  glm_scale(model,     VEC3(width, height, 1));

  canvas_unim4(shader, "MODEL", *model);
  canvas_uni1i(shader, "S_TEX", texture ? (texture - GL_TEXTURE0) : 29);
  canvas_uni3f(shader, "COL",   color[0], color[1], color[2]);

  glBindBuffer(GL_ARRAY_BUFFER, PLANE_VBO);
  glBindVertexArray(PLANE_VAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Text

typedef struct {
  u32 texture, size;
  i16 spacing;
  f32 ratio;
} Font;

void hud_draw_text(u32 shader, char* text, i32 x, i32 y, Font font, vec3 color) {
  canvas_uni1i(shader, "TILE_AMOUNT", 95);

  for (u8 i = 0; text[i]; i++) {
    canvas_uni1i(shader, "TILE", text[i] - 32);
    hud_draw_rec(shader, font.texture, color, x + (font.size + font.spacing) * i, y, font.size, font.size * font.ratio);
  }

  canvas_uni1i(shader, "TILE_AMOUNT", 0);
  canvas_uni1i(shader, "TILE", 0);
}

f32 canvas_text_width(char* text, Font font, f32 size) {
    u32 i = 0;
    while (text[i]) i++;
    return i * (1 + (f32) font.spacing / font.size) * font.size * size;
}

void canvas_draw_text(u32 shader, char* text, f32 x, f32 y, f32 z, f32 size, Font font, Material material, vec3 rotation) {
  glDisable(GL_CULL_FACE);
  canvas_set_material(shader, material);
  canvas_uni1i(shader, "MAT.S_DIF", font.texture ? (font.texture - GL_TEXTURE0) : 29);
  canvas_uni1i(shader, "TILE_AMOUNT", 95);

  mat4 model;
  glm_mat4_identity(model);
  glm_translate(model, VEC3(x, y, z));
  glm_rotate(model, rotation[0], VEC3(1, 0, 0));
  glm_rotate(model, rotation[1], VEC3(0, 1, 0));
  glm_rotate(model, rotation[2], VEC3(0, 0, 1));
  glm_scale(model,     VEC3(font.size * size, font.size * font.ratio * size, 1));

  for (u8 i = 0; text[i]; i++) {
    canvas_uni1i(shader, "TILE", text[i] - 32);


    glBindBuffer(GL_ARRAY_BUFFER, PLANE_VBO);
    glBindVertexArray(PLANE_VAO);
    canvas_unim4(shader, "MODEL", *model);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glm_translate(model, VEC3(1 + (font.spacing * size) / (font.size * size), 0, 0));
  }

  canvas_uni1i(shader, "TILE_AMOUNT", 0);
  canvas_uni1i(shader, "TILE", 0);
  glEnable(GL_CULL_FACE);
}

// Audio

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

typedef struct {
  ma_sound sound;
  c8 name[32];
} Sound;

ma_engine engine;
Sound sounds[MAX_SOUNDS];
u8 sounds_size = 0;

void init_audio_engine() {
  ASSERT(ma_engine_init(NULL, &engine) == MA_SUCCESS, "Failed to init audio");

  struct dirent* de;
  DIR* dir = opendir("./wav");
  ASSERT(dir, "Could not open \"wav\" directory");

  while ((de = readdir(dir)) != NULL) {
    if (de->d_type != DT_REG) continue;
    c8 buffer[64] = { 0 };
    sprintf(buffer, "wav/%s", de->d_name);
    ASSERT(ma_sound_init_from_file(&engine, buffer, 0, NULL, NULL, &sounds[sounds_size].sound) == MA_SUCCESS, "Failed to load %s", de->d_name);
    strcpy(sounds[sounds_size++].name, de->d_name);
  }
}

void play_audio(c8* name) {
  c8 buffer[64] = { 0 };
  sprintf(buffer, "%s.wav", name);
  for (int i = 0; i < sounds_size; i++) {
    if (strcmp(sounds[i].name, buffer)) continue;
    ma_sound_start(&sounds[i].sound);
    return;
  }

  EXIT("Audio \"%s\" not found", name);
}

// Camera

void camera_compute_movement(Camera* cam, u8 shader) {
  vec3 prompted_move = {
    (glfwGetKey(cam->window, GLFW_KEY_D) == GLFW_PRESS ? cam->speed / cam->fps : 0) + (glfwGetKey(cam->window, GLFW_KEY_A) == GLFW_PRESS ? -cam->speed / cam->fps : 0),
    (glfwGetKey(cam->window, GLFW_KEY_E) == GLFW_PRESS ? cam->speed / cam->fps : 0) + (glfwGetKey(cam->window, GLFW_KEY_Q) == GLFW_PRESS ? -cam->speed / cam->fps : 0),
    (glfwGetKey(cam->window, GLFW_KEY_W) == GLFW_PRESS ? cam->speed / cam->fps : 0) + (glfwGetKey(cam->window, GLFW_KEY_S) == GLFW_PRESS ? -cam->speed / cam->fps : 0)
  };

  if (prompted_move[0] || prompted_move[1] || prompted_move[2]) {
    vec3 lateral  = { 0, 0, 0 };
    glm_vec3_scale(cam->rig, prompted_move[0], lateral);
    vec3 frontal  = { 0, 0, 0 };
    glm_vec3_scale(cam->dir, prompted_move[2], frontal);
    vec3 vertical = { 0, prompted_move[1], 0 };

    glm_vec3_add(cam->pos, lateral,  cam->pos);
    glm_vec3_add(cam->pos, frontal,  cam->pos);
    glm_vec3_add(cam->pos, vertical, cam->pos);
    glUseProgram(shader);
    generate_view_mat(cam, shader);
  };
}

void camera_compute_direction(Camera* cam, u8 shader) {
  static vec2 mouse;
  f64 x, y;
  glfwGetCursorPos(cam->window, &x, &y);

  if (!mouse[0]) { mouse[0] = x; mouse[1] = y; }
  if (x == mouse[0] && y == mouse[1]) return;

  cam->yaw  += (x - mouse[0]) * cam->sensitivity;
  cam->pitch = CLAMP(-cam->camera_lock, cam->pitch + (mouse[1] - y) * cam->sensitivity, cam->camera_lock);

  glm_vec3_copy(VEC3(cos(cam->yaw - PI2) * cos(cam->pitch), sin(cam->pitch), sin(cam->yaw - PI2) * cos(cam->pitch)), cam->dir);
  glm_vec3_copy(VEC3(cos(cam->yaw) * cos(cam->pitch), 0, sin(cam->yaw) * cos(cam->pitch)), cam->rig);
  glm_normalize(cam->rig);

  glUseProgram(shader);
  generate_view_mat(cam, shader);
  mouse[0] = x;
  mouse[1] = y;
}

void camera_handle_inputs(Camera* cam, u32 shader) {
  if (glfwGetKey(cam->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(cam->window, 1);
  camera_compute_movement(cam, shader);
  camera_compute_direction(cam, shader);
}

// Entity

typedef struct {
  Model* model;
  vec3 pos, rot;
} Entity;

Entity* entities[MAX_ENTITIES];
u8 entities_size = 0;

Entity* entity_create(Model* model) {
  Entity* entity = malloc(sizeof(Entity));
  entity->model = model;
  VEC3_COPY(VEC3(0, 0, 0), entity->pos);
  VEC3_COPY(VEC3(0, 0, 0), entity->rot);
  entities[entities_size++] = entity;
  return entity;
}

void entity_draw(Entity e, u32 shader) {
  model_bind(e.model, shader);
  glm_translate(e.model->model, e.pos);
  glm_rotate(e.model->model, e.rot[0], VEC3(1, 0, 0));
  glm_rotate(e.model->model, e.rot[1], VEC3(0, 1, 0));
  glm_rotate(e.model->model, e.rot[2], VEC3(0, 0, 1));
  model_draw(e.model, shader);
}

void entity_draw_all(u32 shader) {
  for (u8 i = 0; i < entities_size; i++)
    entity_draw(*entities[i], shader);
}
