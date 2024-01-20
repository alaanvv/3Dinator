#include "canvas.h"

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, y, z) (MAX(MIN(z, y), x))
#define CIRCULAR_CLAMP(x, y, z) (y < x ? z : y > z ? x : y)

#define PI  3.14159
#define TAU PI * 2
#define PI2 PI / 2
#define PI4 PI / 4

#define WIDTH  1920
#define HEIGHT 1080
#define SPEED 0.1 
#define SENSITIVITY 0.001
#define CAMERA_LOCK PI4 * 0.99
#define FOV PI4
#define NEAR 0.1
#define FAR  100

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;

void set_material(u32 shader, vec3 col, vec3 amb, vec3 dif, vec3 spc, i32 s_dif, i32 s_spc, i32 s_emt, f32 shi, i32 use_s_dif, i32 use_s_spc, i32 use_s_emt, vec2 tex_scale, vec2 tex_innset, vec2 tex_outset);
void key_callback(GLFWwindow*, i32, i32, i32, i32);
void mouse_callback(GLFWwindow*, f64, f64);
void handle_inputs(GLFWwindow*);

// --- Setup

#include "mesh.h" 

Canvas canvas = { NULL, WIDTH, HEIGHT };
Camera cam = { WIDTH, HEIGHT, FOV, NEAR, FAR, { 0, 5, 5 }, { 0, 0, -1 }, { 1, 0, 0 } };

mat4 model, view, proj;
f32 last_mouse_x, last_mouse_y;
u32 shader_obj, shader_lig;

vec3 lig = { 0.5, 0.0, 0.8 };
vec3 col = { 1.0, 1.0, 1.0 };
vec3 amb = { 0.4, 0.4, 0.4 };
vec3 dif = { 0.5, 0.5, 0.5 };

u8 toggle;


// --- New stuff
/*
typedef struct Vertex {
  vec3 Pos;
  vec3 Nrm;
  vec2 Tex;
};

typedef struct Texture {
  u8 id;
  char type[];
};

typedef struct {
  Vertex vertices[];
  Texture textures[];
  u8 vertices[], VAO, VBO, EBO;
} Mesh;

mesh_draw(Shader &shader);

void mesh_setup() {
  // TODO Use my own
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u8), &indices[0], GL_STATIC_DRAW);

  canvas_vertex_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);
  canvas_vertex_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Nrm));
  canvas_vertex_attrib_pointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Tex));
}

Vertex vertex = { (vec3) { 0.2, 0.4, 0.6 }, (vec3) {0, 1, 0}, (vec2) { 1, 0 } };
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices[0], GL_STATIC_DRAW);    
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Normal));  

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

void mesh_draw(u8 shader) {
  u8 diffuseNr = 1;
  u8 specularNr = 1;
  for (u8 i = 0; i < textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    char number[];
    cahr name[] = textures[i].type;
    if (name == "texture_diffuse")
      number = std::to_string(diffuseNr++);
    else if(name == "texture_specular")
      number = std::to_string(specularNr++);

    shader.setInt(("material." + name + number).c_str(), i);
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }
  glActiveTexture(GL_TEXTURE0);

  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
} 
*/

// --- Main

void main() {
  canvas_init(&canvas, "Light", (CanvasInitConfig) { GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_LINEAR, GL_LINEAR, (RGBA) { 0, 0, 0.07, 1 }, 1, 1, 1, key_callback, mouse_callback });

  u32 VAO = canvas_create_VAO();
  u32 VBO = canvas_create_VBO(sizeof(cube), cube, GL_STATIC_DRAW);

  canvas_vertex_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) 0);
  canvas_vertex_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (3 * sizeof(f32)));
  canvas_vertex_attrib_pointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (6 * sizeof(f32)));

  shader_lig = shader_create_program("shd/obj.v", "shd/lig.f");
  shader_obj = shader_create_program("shd/obj.v", "shd/obj.f");

  canvas_create_texture(GL_TEXTURE0, "img/wooden-floor.ppm");
  canvas_create_texture(GL_TEXTURE1, "img/wall.ppm");
  canvas_create_texture(GL_TEXTURE2, "img/pumpkin.ppm");
  canvas_create_texture(GL_TEXTURE3, "img/posters.ppm");
  canvas_create_texture(GL_TEXTURE4, "img/pumpkin-specular.ppm");
  canvas_create_texture(GL_TEXTURE5, "img/door.ppm");

  canvas_uni3f(shader_obj, "SPT_LIGS[0].COL", 1, 1, 1);
  canvas_uni3f(shader_obj, "SPT_LIGS[0].POS", 0, 0, 0);
  canvas_uni1f(shader_obj, "SPT_LIGS[0].CON", 1);
  canvas_uni1f(shader_obj, "SPT_LIGS[0].LIN", 0.2);
  canvas_uni1f(shader_obj, "SPT_LIGS[0].QUA", 0.004);
  canvas_uni1f(shader_obj, "SPT_LIGS[0].INN", cos(0.21));
  canvas_uni1f(shader_obj, "SPT_LIGS[0].OUT", cos(0.25));
  canvas_uni3f(shader_obj, "PNT_LIGS[0].COL", lig[0], lig[1], lig[2]);
  canvas_uni3f(shader_obj, "PNT_LIGS[0].POS", 0, 6.9, 0);
  canvas_uni1f(shader_obj, "PNT_LIGS[0].CON", 1);
  canvas_uni1f(shader_obj, "PNT_LIGS[0].LIN", 0.07);
  canvas_uni1f(shader_obj, "PNT_LIGS[0].QUA", 0.017);

  generate_proj_mat(cam, proj);
  generate_view_mat(cam, view);

  while (!glfwWindowShouldClose(canvas.window)) {
    // Draw lights
    glUseProgram(shader_lig);
    canvas_unim4(shader_lig, "PROJ", proj[0]);
    canvas_unim4(shader_lig, "VIEW", view[0]);

    glm_mat4_identity(model);
    glm_translate(model, (vec3) { 0, 6.9, 0 });
    glm_scale(model, (vec3) { 0.3, 0.3, 0.3 });
    canvas_uni3f(shader_lig, "COL", lig[0], lig[1], lig[2]);
    canvas_unim4(shader_lig, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Draw objects
    glUseProgram(shader_obj);
    canvas_unim4(shader_obj, "PROJ", proj[0]);
    canvas_unim4(shader_obj, "VIEW", view[0]);
    canvas_uni3f(shader_obj, "CAM", cam.pos[0], cam.pos[1], cam.pos[2]);
    canvas_uni3f(shader_obj, "SPT_LIGS[0].POS", cam.pos[0], cam.pos[1], cam.pos[2]);
    canvas_uni3f(shader_obj, "SPT_LIGS[0].DIR", cam.dir[0], cam.dir[1], cam.dir[2]);

    // Pumpkin
    set_material(shader_obj, col, amb, dif, (vec3) { 0.3, 0.3, 0.3 }, 2, 4, 0, 51, 1, 1, 0, (vec2) { 1, 1 }, (vec2) {}, (vec2) {});
    glm_mat4_identity(model);
    glm_translate(model, (vec3) { 9, 0.5, -9 });
    glm_rotate(model, -PI4 / 2, (vec3) { 0, 1, 0 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Room floor
    set_material(shader_obj, col, amb, dif, (vec3) { 0.5, 0.5, 0.5 }, 0, 0, 0, 51, 1, 0, 0, (vec2) { 18, 3 }, (vec2) {}, (vec2) {});
    glm_mat4_identity(model);
    glm_scale(model, (vec3) { 20, 0, 20 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Room ceil
    set_material(shader_obj, col, amb, dif, (vec3) {}, 1, 0, 0, 51, 1, 0, 0, (vec2) { 48, 8 }, (vec2) {}, (vec2) {});
    glm_mat4_identity(model);
    glm_translate(model, (vec3) { 0, 7, 0 });
    glm_scale(model, (vec3) { 20, 0, 20 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 6, 6);

    // Room walls
    set_material(shader_obj, col, amb, dif, (vec3) {}, 1, 0, 0, 51, 1, 0, 0, (vec2) { 8 * 20 / 7 * 6, 8 }, (vec2) {}, (vec2) {});
    glm_mat4_identity(model);
    glm_translate(model, (vec3) { 0, 3.5, -10 });
    glm_scale(model, (vec3) { 20, 7, 0 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    glm_mat4_identity(model);
    glm_translate(model, (vec3) { 0, 3.5, 10 });
    glm_scale(model, (vec3) { 20, 7, 0 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 18, 6);

    glm_mat4_identity(model);
    glm_translate(model, (vec3) { -10, 3.5, 0 });
    glm_scale(model, (vec3) { 0, 7, 20 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 24, 6);

    glm_mat4_identity(model);
    glm_translate(model, (vec3) { 10, 3.5, 0 });
    glm_scale(model, (vec3) { 0, 7, 20 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 30, 6);

    // Posters
    set_material(shader_obj, col, amb, dif, (vec3) { 0.5, 0.5, 0.5 }, 3, 0, 0, 51, 1, 0, 0, (vec2) {}, (vec2) {}, (vec2) {});
    glm_mat4_identity(model);
    glm_translate(model, (vec3) { -4, 3.5, -9.999 });
    glm_rotate(model, PI2 * 0.03, (vec3) { 0, 0, 1 });
    glm_scale(model, (vec3) { 2.5, 3.5, 0 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    set_material(shader_obj, col, amb, dif, (vec3) { 0.5, 0.5, 0.5 }, 3, 0, 0, 51, 1, 0, 0, (vec2) {}, (vec2) { (f32) 1 / 6 }, (vec2) {});
    glm_mat4_identity(model);
    glm_translate(model, (vec3) { 2, 3.5, -9.998 });
    glm_rotate(model, PI2 * -0.003, (vec3) { 0, 0, 1 });
    glm_scale(model, (vec3) { 2.5 * 0.8, 3.5 * 0.8, 0 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    set_material(shader_obj, col, amb, dif, (vec3) { 0.5, 0.5, 0.5 }, 3, 0, 0, 51, 1, 0, 0, (vec2) {}, (vec2) { (f32) 2 / 6 }, (vec2) {});
    glm_mat4_identity(model);
    glm_translate(model, (vec3) { 6.5, 1.5, -9.997 });
    glm_rotate(model, PI2 * 0.1, (vec3) { 0, 0, 1 });
    glm_scale(model, (vec3) { 2.5 * 0.5, 3.5 * 0.6, 0 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    set_material(shader_obj, col, amb, dif, (vec3) { 0.5, 0.5, 0.5 }, 3, 0, 0, 51, 1, 0, 0, (vec2) {}, (vec2) { (f32) 3 / 6 }, (vec2) {});
    glm_mat4_identity(model);
    glm_translate(model, (vec3) { 5, 3, -9.996 });
    glm_rotate(model, PI2 * 0.05, (vec3) { 0, 0, 1 });
    glm_scale(model, (vec3) { 2.5 * 0.75, 3.5 * 0.75, 0 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    set_material(shader_obj, col, amb, dif, (vec3) { 0.5, 0.5, 0.5 }, 3, 0, 0, 51, 1, 0, 0, (vec2) {}, (vec2) { (f32) 4 / 6 }, (vec2) {});
    glm_mat4_identity(model);
    glm_translate(model, (vec3) { -0.2, 2.5, -9.995 });
    glm_rotate(model, PI2 * 0.02, (vec3) { 0, 0, 1 });
    glm_scale(model, (vec3) { 2.5, 3.5, 0 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    set_material(shader_obj, col, amb, dif, (vec3) { 0.5, 0.5, 0.5 }, 3, 0, 0, 51, 1, 0, 0, (vec2) {}, (vec2) { (f32) 5 / 6 }, (vec2) {});
    glm_mat4_identity(model);
    glm_translate(model, (vec3) { -6.16, 4, -9.995 });
    glm_rotate(model, PI2 * 0.02, (vec3) { 0, 0, 1 });
    glm_scale(model, (vec3) { 2.5 * 1, 3.5 * 1, 0 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    // Door
    set_material(shader_obj, col, amb, dif, (vec3) { 0.6, 0.6, 0.6 }, 5, 0, 0, 51, 1, 0, 0, (vec2) { 6, 1 }, (vec2) {}, (vec2) {});
    glm_mat4_identity(model);
    glm_translate(model, (vec3) { -7, 3, 9.999 });
    glm_scale(model, (vec3) { 3, 6, 0 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 18, 6);

    // Finish
    glfwSwapBuffers(canvas.window); 
    glfwPollEvents();
    handle_inputs(canvas.window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  glfwTerminate();
}

// --- Function

void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
  if (action != GLFW_PRESS) return;

  if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, 1); 
  if (key == GLFW_KEY_F) toggle = 1 - toggle;
}

void handle_inputs(GLFWwindow* window) {
  inline void move_cam(vec3 dir) {
    if (dir[0] || dir[2]) {
      vec3 move;
      glm_vec3_scale(dir[0] ? cam.rig : cam.dir, dir[0] ? dir[0] : dir[2], move);
      glm_vec3_add(cam.pos, move, cam.pos);
    }
    else cam.pos[1] += dir[1];
    generate_view_mat(cam, view);
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move_cam((vec3) {  SPEED,  0,  0 });
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move_cam((vec3) { -SPEED,  0,  0 });
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) move_cam((vec3) {  0,  SPEED,  0 });
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) move_cam((vec3) {  0, -SPEED,  0 });
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move_cam((vec3) {  0,  0,  SPEED });
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move_cam((vec3) {  0,  0, -SPEED });

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) { cam.fov = MIN(cam.fov + PI / 100, FOV); generate_proj_mat(cam, proj); }
  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) { cam.fov = MAX(cam.fov - PI / 100, 0.1); generate_proj_mat(cam, proj); }
}

void mouse_callback(GLFWwindow* window, f64 x, f64 y) {
  if (last_mouse_x == 0) {
    last_mouse_x = x;
    last_mouse_y = y;
    return;
  }

  f64 offset_x = (x - last_mouse_x) * SENSITIVITY;
  f64 offset_y = (last_mouse_y - y) * SENSITIVITY;
  last_mouse_x = x;
  last_mouse_y = y;

  cam.yaw  += offset_x;
  cam.pitch = CLAMP(-CAMERA_LOCK, cam.pitch + offset_y, CAMERA_LOCK);

  cam.dir[0] = cos(cam.yaw - PI2) * cos(cam.pitch);
  cam.dir[1] = sin(cam.pitch);
  cam.dir[2] = sin(cam.yaw - PI2) * cos(cam.pitch);

  cam.rig[0] = cos(cam.yaw) * cos(cam.pitch);
  cam.rig[2] = sin(cam.yaw) * cos(cam.pitch);

  generate_view_mat(cam, view);
}

void set_material(u32 shader, vec3 col, vec3 amb, vec3 dif, vec3 spc, i32 s_dif, i32 s_spc, i32 s_emt, f32 shi, i32 use_s_dif, i32 use_s_spc, i32 use_s_emt, vec2 tex_scale, vec2 tex_innset, vec2 tex_outset) {
  canvas_uni3f(shader, "MAT.COL", col[0], col[1], col[2]);
  canvas_uni3f(shader, "MAT.AMB", amb[0], amb[1], amb[2]);
  canvas_uni3f(shader, "MAT.DIF", dif[0], dif[1], dif[2]);
  canvas_uni3f(shader, "MAT.SPC", spc[0], spc[1], spc[2]);
  canvas_uni1i(shader, "MAT.S_DIF", s_dif);
  canvas_uni1i(shader, "MAT.S_SPC", s_spc);
  canvas_uni1i(shader, "MAT.S_EMT", s_emt);
  canvas_uni1i(shader, "MAT.USE_S_DIF", use_s_dif);
  canvas_uni1i(shader, "MAT.USE_S_SPC", use_s_spc);
  canvas_uni1i(shader, "MAT.USE_S_EMT", use_s_emt);
  canvas_uni1f(shader, "MAT.SHI", shi);
  canvas_uni2f(shader, "TEX_SCALE", tex_scale[0], tex_scale[1]);
  canvas_uni2f(shader, "TEX_INNSET", tex_innset[0], tex_innset[1]);
  canvas_uni2f(shader, "TEX_OUTSET", tex_outset[0], tex_outset[1]);
}
