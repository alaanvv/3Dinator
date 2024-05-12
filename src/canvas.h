#include <stdio.h>
#include <stdlib.h>
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
typedef char     c8;

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
} TextureConfig;

typedef struct {
  vec3 col;
  f64  amb, dif, spc, shi;
  i32 s_dif, s_spc, s_emt;
  u8 lig;
} Material;

typedef struct {
  u8  stage, size;
  f32 pos;
} Animation;

typedef struct {
  u8 capture_mouse;
  char* title;
} CanvasInitConfig;

typedef f32 Vertex[8];

typedef struct {
  u32 size, VAO, VBO;
  Vertex* vertexes;
  mat4 model;
} Model;

TextureConfig TEXTURE_DEFAULT = { GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_NEAREST, GL_NEAREST };

// --- Function

void canvas_init(Canvas* canvas, CanvasInitConfig config) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  canvas->window = glfwCreateWindow(canvas->width, canvas->height, config.title, NULL, NULL);
  glfwMakeContextCurrent(canvas->window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glViewport(0, 0, canvas->width, canvas->height);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.2, 0.2, 0.2, 1);

  if (config.capture_mouse) glfwSetInputMode(canvas->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

u32 canvas_create_texture(GLenum unit, char path[], TextureConfig config) {
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     config.wrap_s);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     config.wrap_t);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.mag_filter);
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
void canvas_unim4(u16 s, char u[], const f32* v)           { glUniformMatrix4fv(UNI(s, u), 1, GL_FALSE, (const f32*) (v)); }

void canvas_set_material(u32 shader, Material mat) {
  canvas_uni3f(shader, "MAT.COL", mat.col[0], mat.col[1], mat.col[2]);
  canvas_uni1f(shader, "MAT.AMB", mat.amb);
  canvas_uni1f(shader, "MAT.DIF", mat.dif);
  canvas_uni1f(shader, "MAT.SPC", mat.spc);
  canvas_uni1f(shader, "MAT.SHI", mat.shi);
  canvas_uni1i(shader, "MAT.S_DIF", mat.s_dif);
  canvas_uni1i(shader, "MAT.S_SPC", mat.s_spc);
  canvas_uni1i(shader, "MAT.S_EMT", mat.s_emt);
  canvas_uni1i(shader, "MAT.LIG", mat.lig);
}

// --- Animation

void animation_run(Animation* anim, f32 rate) {
  anim->pos += rate;
  if (anim->pos < 1) return;

  anim->pos = 0;
  anim->stage += 1;
  if (anim->stage > anim->size)
    anim->stage = 0;
}

// --- Meshes

Vertex* model_parse(const c8* path, u32* size) {
  vec3*   poss = malloc(sizeof(vec3));
  vec3*   nrms = malloc(sizeof(vec3));
  vec2*   texs = malloc(sizeof(vec2));
  Vertex* vrts = malloc(sizeof(Vertex));

  u32 pos_i = 0;
  u32 nrm_i = 0;
  u32 tex_i = 0;
  u32 vrt_i = 0;

  FILE* file = fopen(path, "r");
  c8 buffer[256];
  while (fgets(buffer, 256, file)) {
    if      (buffer[0] == 'v' && buffer[1] == ' ') {
      poss = realloc(poss, sizeof(vec3) * (++pos_i + 1));
      sscanf(buffer, "v  %f %f %f", &poss[pos_i][0], &poss[pos_i][1], &poss[pos_i][2]);
    } 
    else if (buffer[0] == 'v' && buffer[1] == 'n') {
      nrms = realloc(nrms, sizeof(vec3) * (++nrm_i + 1));
      sscanf(buffer, "vn %f %f %f", &nrms[nrm_i][0], &nrms[nrm_i][1], &nrms[nrm_i][2]);
    }
    else if (buffer[0] == 'v' && buffer[1] == 't') {
      texs = realloc(texs, sizeof(vec2) * (++tex_i + 1));
      sscanf(buffer, "vt %f %f",    &texs[tex_i][0], &texs[tex_i][1]);
    }
    else if (buffer[0] == 'f') { 
      i32 v1[] = {  0, -1, 0 };
      i32 v2[] = {  0, -1, 0 };
      i32 v3[] = {  0, -1, 0 };
      i32 v4[] = { -1, -1, 0 };

      sscanf(buffer, "f %d/  /%d %d/  /%d %d/  /%d",          &v1[0],         &v1[2], &v2[0],         &v2[2], &v3[0],         &v3[2]);
      sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d",          &v1[0], &v1[1], &v1[2], &v2[0], &v2[1], &v2[2], &v3[0], &v3[1], &v3[2]);
      sscanf(buffer, "f %d/  /%d %d/  /%d %d/  /%d %d/  /%d", &v1[0],         &v1[2], &v2[0],         &v2[2], &v3[0],         &v3[2], &v4[0],         &v4[2]);
      sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &v1[0], &v1[1], &v1[2], &v2[0], &v2[1], &v2[2], &v3[0], &v3[1], &v3[2], &v4[0], &v4[1], &v4[2]);

      if (v4[0] == -1) { // TRI
        vrts = realloc(vrts, sizeof(Vertex) * (vrt_i + 3)); // I foresee a segfault

        vrts[vrt_i][0] = poss[v1[0]][0];
        vrts[vrt_i][1] = poss[v1[0]][1];
        vrts[vrt_i][2] = poss[v1[0]][2];
        vrts[vrt_i][3] = nrms[v1[2]][0];
        vrts[vrt_i][4] = nrms[v1[2]][1];
        vrts[vrt_i][5] = nrms[v1[2]][2];
        vrts[vrt_i][6] = texs[v1[1]][0];
        vrts[vrt_i][7] = texs[v1[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v2[0]][0];
        vrts[vrt_i][1] = poss[v2[0]][1];
        vrts[vrt_i][2] = poss[v2[0]][2];
        vrts[vrt_i][3] = nrms[v2[2]][0];
        vrts[vrt_i][4] = nrms[v2[2]][1];
        vrts[vrt_i][5] = nrms[v2[2]][2];
        vrts[vrt_i][6] = texs[v2[1]][0];
        vrts[vrt_i][7] = texs[v2[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v3[0]][0];
        vrts[vrt_i][1] = poss[v3[0]][1];
        vrts[vrt_i][2] = poss[v3[0]][2];
        vrts[vrt_i][3] = nrms[v3[2]][0];
        vrts[vrt_i][4] = nrms[v3[2]][1];
        vrts[vrt_i][5] = nrms[v3[2]][2];
        vrts[vrt_i][6] = texs[v3[1]][0];
        vrts[vrt_i][7] = texs[v3[1]][1];
        vrt_i++;
      }
      else {
        vrts = realloc(vrts, sizeof(Vertex) * (vrt_i + 6)); // I foresee a segfault

        vrts[vrt_i][0] = poss[v1[0]][0];
        vrts[vrt_i][1] = poss[v1[0]][1];
        vrts[vrt_i][2] = poss[v1[0]][2];
        vrts[vrt_i][3] = nrms[v1[2]][0];
        vrts[vrt_i][4] = nrms[v1[2]][1];
        vrts[vrt_i][5] = nrms[v1[2]][2];
        vrts[vrt_i][6] = texs[v1[1]][0];
        vrts[vrt_i][7] = texs[v1[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v2[0]][0];
        vrts[vrt_i][1] = poss[v2[0]][1];
        vrts[vrt_i][2] = poss[v2[0]][2];
        vrts[vrt_i][3] = nrms[v2[2]][0];
        vrts[vrt_i][4] = nrms[v2[2]][1];
        vrts[vrt_i][5] = nrms[v2[2]][2];
        vrts[vrt_i][6] = texs[v2[1]][0];
        vrts[vrt_i][7] = texs[v2[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v3[0]][0];
        vrts[vrt_i][1] = poss[v3[0]][1];
        vrts[vrt_i][2] = poss[v3[0]][2];
        vrts[vrt_i][3] = nrms[v3[2]][0];
        vrts[vrt_i][4] = nrms[v3[2]][1];
        vrts[vrt_i][5] = nrms[v3[2]][2];
        vrts[vrt_i][6] = texs[v3[1]][0];
        vrts[vrt_i][7] = texs[v3[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v1[0]][0];
        vrts[vrt_i][1] = poss[v1[0]][1];
        vrts[vrt_i][2] = poss[v1[0]][2];
        vrts[vrt_i][3] = nrms[v1[2]][0];
        vrts[vrt_i][4] = nrms[v1[2]][1];
        vrts[vrt_i][5] = nrms[v1[2]][2];
        vrts[vrt_i][6] = texs[v1[1]][0];
        vrts[vrt_i][7] = texs[v1[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v3[0]][0];
        vrts[vrt_i][1] = poss[v3[0]][1];
        vrts[vrt_i][2] = poss[v3[0]][2];
        vrts[vrt_i][3] = nrms[v3[2]][0];
        vrts[vrt_i][4] = nrms[v3[2]][1];
        vrts[vrt_i][5] = nrms[v3[2]][2];
        vrts[vrt_i][6] = texs[v3[1]][0];
        vrts[vrt_i][7] = texs[v3[1]][1];
        vrt_i++;

        vrts[vrt_i][0] = poss[v4[0]][0];
        vrts[vrt_i][1] = poss[v4[0]][1];
        vrts[vrt_i][2] = poss[v4[0]][2];
        vrts[vrt_i][3] = nrms[v4[2]][0];
        vrts[vrt_i][4] = nrms[v4[2]][1];
        vrts[vrt_i][5] = nrms[v4[2]][2];
        vrts[vrt_i][6] = texs[v4[1]][0];
        vrts[vrt_i][7] = texs[v4[1]][1];
        vrt_i++;
      }
    }
  }
  fclose(file);
  free(poss);
  free(nrms);
  free(texs);

  *size = vrt_i;
  return vrts;
}

Model* model_create(const c8* path) {
  Model* model = malloc(sizeof(Model));
  model->vertexes = model_parse(path, &model->size);

  model->VAO = canvas_create_VAO();
  model->VBO = canvas_create_VBO(model->size * sizeof(Vertex), model->vertexes, GL_STATIC_DRAW);
  canvas_vertex_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) 0);
  canvas_vertex_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (3 * sizeof(f32)));
  canvas_vertex_attrib_pointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (6 * sizeof(f32)));

  return model;
}

void model_draw(Model* model, u32 shader) {
  glBindBuffer(GL_ARRAY_BUFFER, model->VBO);
  glBindVertexArray(model->VAO);
  canvas_unim4(shader, "MODEL", model->model[0]);
  glDrawArrays(GL_TRIANGLES, 0, model->size);
}
