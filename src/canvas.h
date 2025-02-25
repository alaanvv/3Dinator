#include <glad/glad.h>
#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

#define UNI(shd, uni) (glGetUniformLocation(shd, uni))

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, y, z) (MAX(MIN(z, y), x))
#define CIRCULAR_CLAMP(x, y, z) ((y < x) ? z : ((y > z) ? x : y))
#define RAND(min, max) (random() % (max - min) + min)
#define ASSERT(x, ...) if (!(x)) { printf(__VA_ARGS__); exit(1); }
#define PRINT(...) { printf(__VA_ARGS__); printf("\n"); }
#define VEC2_COMPARE(v1, v2) (v1[0] == v2[0] && v1[1] == v2[1])
#define VEC3_COMPARE(v1, v2) (v1[0] == v2[0] && v1[1] == v2[1] && v1[2] == v2[2])

#define PI  3.14159
#define TAU PI * 2
#define PI2 PI / 2
#define PI4 PI / 4

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;
typedef char     c8;

// Canvas

typedef struct {
  f32 fov, near, far;
  vec3 pos, dir, rig;
  f32 pitch, yaw;
  u16 width, height;
  GLFWwindow* window;
  mat4 view, proj;
} Camera;

typedef struct {
  char* title;
  u8 capture_mouse, fullscreen;
  f32 screen_size;
} CanvasInitConfig;

void canvas_init(Camera* cam, CanvasInitConfig config) {
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
  glViewport(0, 0, cam->width, cam->height);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glClearColor(0.0, 0.0, 0.0, 1);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (config.capture_mouse) glfwSetInputMode(cam->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glm_vec3_copy((vec3) { 0, 0, -1 }, cam->dir);
  glm_vec3_copy((vec3) { 1, 0,  0 }, cam->rig);
}

void generate_proj_mat(Camera* cam, u32 shader) {
  glm_mat4_identity(cam->proj);
  glm_perspective(cam->fov, (f32) cam->width / cam->height, cam->near, cam->far, cam->proj);
  glUniformMatrix4fv(UNI(shader, "PROJ"), 1, GL_FALSE, cam->proj[0]);
}

void generate_view_mat(Camera* cam, u32 shader) {
  vec3 target, up;
  glm_cross(cam->rig, cam->dir, up);
  glm_vec3_add(cam->pos, cam->dir, target);
  glm_lookat(cam->pos, target, up, cam->view);
  glUniformMatrix4fv(UNI(shader, "VIEW"), 1, GL_FALSE, cam->view[0]);
}

void use_screen_space(Camera* cam, u32 shader, u8 use) {
  if (!use) {
    glUniformMatrix4fv(UNI(shader, "PROJ"), 1, GL_FALSE, cam->proj[0]);
    glUniformMatrix4fv(UNI(shader, "VIEW"), 1, GL_FALSE, cam->view[0]);
    return;
  }
  mat4 blank;
  glm_mat4_identity(blank);
  glUniformMatrix4fv(UNI(shader, "PROJ"), 1, GL_FALSE, blank[0]);
  glUniformMatrix4fv(UNI(shader, "VIEW"), 1, GL_FALSE, blank[0]);
}

void update_fps(f32* fps) {
  static f32 tick = 0;
  *fps = 1 / (glfwGetTime() - tick);
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

u32 canvas_create_EBO() {
  u32 EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  return EBO;
}

u32 canvas_create_FBO(u16 width, u16 height, GLenum min, GLenum mag) {
  u32 FBO;
  glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	u32 REN_TEX;
	glGenTextures(1, &REN_TEX);
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

u32 _create_shader(GLenum type, char path[], char name[]) {
  FILE* file = fopen(path, "r");
  ASSERT(file, "Can't open %s shader (%s)", name, path);
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
  ASSERT(success, "Error compiling %s shader (%s)", name, path);
  return shader;
}

u32 shader_create_program(char vertex_path[], char fragment_path[]) {
  u32 v_shader = _create_shader(GL_VERTEX_SHADER, vertex_path, "vertex");
  u32 f_shader = _create_shader(GL_FRAGMENT_SHADER, fragment_path, "fragment");
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

void canvas_uni1i(u16 s, char u[], i32 v1)                 { glUniform1i(UNI(s, u), v1); }
void canvas_uni1f(u16 s, char u[], f32 v1)                 { glUniform1f(UNI(s, u), v1); }
void canvas_uni2i(u16 s, char u[], i32 v1, i32 v2)         { glUniform2i(UNI(s, u), v1, v2); }
void canvas_uni2f(u16 s, char u[], f32 v1, f32 v2)         { glUniform2f(UNI(s, u), v1, v2); }
void canvas_uni3i(u16 s, char u[], i32 v1, i32 v2, i32 v3) { glUniform3i(UNI(s, u), v1, v2, v3); }
void canvas_uni3f(u16 s, char u[], f32 v1, f32 v2, f32 v3) { glUniform3f(UNI(s, u), v1, v2, v3); }
void canvas_unim4(u16 s, char u[], const f32* m)           { glUniformMatrix4fv(UNI(s, u), 1, GL_FALSE, m); }

// Texture

typedef struct {
  GLenum wrap_s, wrap_t, min_filter, mag_filter;
} TextureConfig;

TextureConfig TEXTURE_DEFAULT = { GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_NEAREST, GL_NEAREST };

u32 canvas_create_texture(GLenum unit, char path[], TextureConfig config) {
  FILE* img = fopen(path, "r");
  ASSERT(img, "Can't open image (%s)", path);

  u16 ppm, width, height, max_color;
  fscanf(img, "P%hi %hi %hi %hi", &ppm, &width, &height, &max_color);
  ASSERT(ppm == 3, "Not a PPM3 (%s)", path);

  f32* buffer = malloc(sizeof(f32) * width * height * 3);
  for (u32 i = 0; i < width * height * 3; i += 3) {
    fscanf(img, "%f %f %f", &buffer[i], &buffer[i + 1], &buffer[i + 2]);
    glm_vec3_scale(&buffer[i], (f32) 1 / max_color, &buffer[i]);
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

// Material

typedef struct {
  vec3 col;
  f64  amb, dif;
  u8   s_dif, s_emt, lig, png;
} Material;

void canvas_set_material(u32 shader, Material mat) {
  canvas_uni3f(shader, "MAT.COL", mat.col[0], mat.col[1], mat.col[2]);
  canvas_uni1f(shader, "MAT.AMB", mat.amb);
  canvas_uni1f(shader, "MAT.DIF", mat.dif);
  canvas_uni1i(shader, "MAT.S_DIF", mat.s_dif);
  canvas_uni1i(shader, "MAT.S_EMT", mat.s_emt);
  canvas_uni1i(shader, "MAT.LIG", mat.lig);
  canvas_uni1i(shader, "MAT.PNG", mat.png);
}

// Animation

typedef struct {
  u8  stage, size;
  f32 pos;
} Animation;

void animation_start(Animation* anim) {
  anim->stage = 1;
}

u8 animation_run(Animation* anim, f32 rate) {
  if (!anim->stage) return 0;
  anim->pos += rate;
  if (anim->pos < 1) return 0;

  anim->pos = 0;
  anim->stage += 1;
  if (anim->stage <= anim->size) return 0;
  anim->stage = 0;
  return 1;
}

// Model

typedef f32 Vertex[8];

typedef struct {
  u32 size, VAO, VBO;
  Vertex* vertexes;
  mat4 model;
  Material* material;
} Model;

void model_parse(Model* model, const c8* path, u32* size, f32 scale) {
  vec3*   poss = malloc(sizeof(vec3));
  vec2*   texs = malloc(sizeof(vec2));
  Vertex* vrts = malloc(sizeof(Vertex));

  u32 pos_i = 0;
  u32 tex_i = 0;
  u32 vrt_i = 0;

  FILE* file = fopen(path, "r");
  c8 buffer[256];
  while (fgets(buffer, 256, file)) {
    if      (buffer[0] == 'v' && buffer[1] == ' ') {
      poss = realloc(poss, sizeof(vec3) * (++pos_i + 1));
      sscanf(buffer, "v  %f %f %f", &poss[pos_i][0], &poss[pos_i][1], &poss[pos_i][2]);
      glm_vec3_scale(poss[pos_i], scale, poss[pos_i]);
    }
    else if (buffer[0] == 'v' && buffer[1] == 't') {
      texs = realloc(texs, sizeof(vec2) * (++tex_i + 1));
      sscanf(buffer, "vt %f %f",    &texs[tex_i][0], &texs[tex_i][1]);
    }
    else if (buffer[0] == 'f') {
      u32 vs[4][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };

      sscanf(buffer, "f %d       %d       %d",                &vs[0][0],                       &vs[1][0],                       &vs[2][0]);
      sscanf(buffer, "f %d/  /%d %d/  /%d %d/  /%d",          &vs[0][0],            &vs[0][2], &vs[1][0],            &vs[1][2], &vs[2][0],            &vs[2][2]);
      sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d",          &vs[0][0], &vs[0][1], &vs[0][2], &vs[1][0], &vs[1][1], &vs[1][2], &vs[2][0], &vs[2][1], &vs[2][2]);
      sscanf(buffer, "f %d       %d       %d       %d",       &vs[0][0],                       &vs[1][0],                       &vs[2][0],                       &vs[3][0]);
      sscanf(buffer, "f %d/  /%d %d/  /%d %d/  /%d %d/  /%d", &vs[0][0],            &vs[0][2], &vs[1][0],            &vs[1][2], &vs[2][0],            &vs[2][2], &vs[3][0],            &vs[3][2]);
      sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &vs[0][0], &vs[0][1], &vs[0][2], &vs[1][0], &vs[1][1], &vs[1][2], &vs[2][0], &vs[2][1], &vs[2][2], &vs[3][0], &vs[3][1], &vs[3][2]);

      vec3 side_1, side_2, nrm;
      glm_vec3_sub(poss[vs[1][0]], poss[vs[0][0]], side_1);
      glm_vec3_sub(poss[vs[2][0]], poss[vs[0][0]], side_2);
      glm_vec3_cross(side_1, side_2, nrm);
      glm_vec3_normalize(nrm);

      vrts = realloc(vrts, sizeof(Vertex) * (vrt_i + 3));
      for (u8 i = 0; i < 3; i++) {
        glm_vec3_copy(poss[vs[i][0]], &vrts[vrt_i][0]);
        glm_vec3_copy(nrm,            &vrts[vrt_i][3]);
        glm_vec2_copy(texs[vs[i][1]], &vrts[vrt_i][6]);
        vrt_i++;
      }

      if (!vs[3][0]) continue;
      vrts = realloc(vrts, sizeof(Vertex) * (vrt_i + 3));

      for (u8 i = 1; i < 4; i++) {
        glm_vec3_copy(poss[vs[i == 1 ? 0 : i][0]], &vrts[vrt_i][0]);
        glm_vec3_copy(nrm,                         &vrts[vrt_i][3]);
        glm_vec2_copy(texs[vs[i == 1 ? 0 : i][1]], &vrts[vrt_i][6]);
        vrt_i++;
      }
    }
  }

  fclose(file);
  free(poss);
  free(texs);

  *size = vrt_i;
  model->vertexes = vrts;
}

Model* model_create(const c8* path, Material* material, f32 scale) {
  Model* model = malloc(sizeof(Model));
  model_parse(model, path, &model->size, scale);
  model->material = material;

  model->VAO = canvas_create_VAO();
  model->VBO = canvas_create_VBO(model->size * sizeof(Vertex), model->vertexes, GL_STATIC_DRAW);
  canvas_vertex_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) 0);
  canvas_vertex_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (3 * sizeof(f32)));
  canvas_vertex_attrib_pointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (6 * sizeof(f32)));

  return model;
}

void model_bind(Model* model, u32 shader) {
  if (model->material != NULL) canvas_set_material(shader, *model->material);
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
