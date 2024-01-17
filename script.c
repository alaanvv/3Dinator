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

void key_callback(GLFWwindow*, i32, i32, i32, i32);
void mouse_callback(GLFWwindow*, f64, f64);
void handle_inputs(GLFWwindow*);

// --- Setup

#include "mesh.h" 

Canvas canvas = { NULL, WIDTH, HEIGHT };
Camera cam = { WIDTH, HEIGHT, FOV, NEAR, FAR, { 0, 0, 15 }, { 0, 0, -1 }, { 1, 0, 0 } };

mat4 model, view, proj;
f32 last_mouse_x, last_mouse_y;
u32 shader_obj, shader_lig;

u8 toggle;

// --- Main

void main() {
  canvas_init(&canvas, "Light", (CanvasInitConfig) { GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST, (RGBA) { 0, 0, 0.07, 1 }, 1, 1, 1, key_callback, mouse_callback });

  u32 VAO = canvas_create_VAO();
  u32 VBO = canvas_create_VBO(sizeof(cube), cube, GL_STATIC_DRAW);

  canvas_vertex_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) 0);
  canvas_vertex_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (3 * sizeof(f32)));
  canvas_vertex_attrib_pointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (6 * sizeof(f32)));

  shader_lig = shader_create_program("shd/obj.v", "shd/lig.f");
  shader_obj = shader_create_program("shd/obj.v", "shd/obj.f");
  canvas_create_texture(GL_TEXTURE0, "img/flr.ppm", shader_obj, "MAT.S_DIF", 0);
  canvas_create_texture(GL_TEXTURE1, "img/pmk.ppm", shader_obj, "MAT.S_DIF", 1);
  canvas_create_texture(GL_TEXTURE2, "img/spc.ppm", shader_obj, "MAT.S_SPC", 2);

  canvas_uni3f(shader_obj, "MAT.COL", 1, 1, 1);
  canvas_uni3f(shader_obj, "MAT.AMB", 0.10, 0.10, 0.10);
  canvas_uni3f(shader_obj, "MAT.DIF", 1.00, 1.00, 1.00);
  canvas_uni3f(shader_obj, "MAT.SPC", 0.50, 0.50, 0.50);
  canvas_uni1f(shader_obj, "MAT.SHI", 51);
  canvas_uni3f(shader_obj, "SPT_LIGS[0].COL", 1, 1, 1);
  canvas_uni3f(shader_obj, "SPT_LIGS[0].POS", 0, 0, 0);
  canvas_uni1f(shader_obj, "SPT_LIGS[0].CON", 1);
  canvas_uni1f(shader_obj, "SPT_LIGS[0].LIN", 0.07);
  canvas_uni1f(shader_obj, "SPT_LIGS[0].QUA", 0.4);
  canvas_uni1f(shader_obj, "SPT_LIGS[0].INN", cos(0.21));
  canvas_uni1f(shader_obj, "SPT_LIGS[0].OUT", cos(0.25));
  canvas_uni1i(shader_obj, "MAT.USE_S_DIF", 1);
  canvas_uni1i(shader_obj, "MAT.USE_S_SPC", 1);

  generate_proj_mat(cam, proj);
  generate_view_mat(cam, view);

  vec3 poss[80] = { {-1, 15, 3}, {0, -3, -14}, {-6, 13, -9}, {5, 6, 10}, {-2, 11, 4}, {-9, 14, -6}, {1, 7, -5}, {0, 18, -3}, {-3, 5, 1}, {8, 14, 1}, {-2, 0, 6}, {0, 10, 6}, {-4, 1, -5}, {6, 2, 5}, {-9, 4, 8}, {-2, 12, -8}, {8, 11, 4}, {6, 15, -5}, {0, 1, -7}, {-10, 16, -3}, {-8, 7, 2}, {5, 7, 8}, {2, 13, 8}, {9, 6, -7}, {-1, 2, -8}, {4, 7, 4}, {-3, 1, 8}, {-2, 3, -3}, {5, 8, -9}, {9, 0, -1}, {-7, 7, -7}, {7, 2, 0}, {15, 3, -10}, {-12, -18, 11}, {17, -16, -18}, {-2, -5, 0}, {14, -7, -20}, {8, -1, 6}, {-18, 2, 3}, {-13, 11, -4}, {-5, -10, 18}, {9, 5, 2}, {10, -12, -10}, {6, 8, -15}, {-1, -8, 10}, {-6, -11, 10}, {12, -15, -12}, {14, -3, -2}, {-15, 20, 14}, {18, 14, 5}, {10, -5, -3}, {-7, -9, 10}, {-17, 15, -14}, {3, -6, -8}, {5, -20, -10}, {0, 10, -18}, {1, -13, 7}, {16, -4, -6}, {4, -14, -2}, {3, 7, -1}, {-20, -19, -8}, {10, -17, 7}, {-14, -2, -13}, {-15, 9, 19}, {-11, -16, 0}, {-19, 4, 16}, {19, -20, 1}, {13, 15, -9}, {7, -7, 10}, {-16, -15, -5}, {2, 16, -3}, {11, 12, 10}, {-4, -3, 11}, {16, -18, 9}, {20, -9, 16}, {-19, -20, -10}, {-6, 0, -8}, {12, 8, -14}, {-8, 17, 18}, {15, 1, -18} };
  vec3 pnt_ligs[] = { { 3, -10 }, { 3, -10 }, { 3, -10 } };

  glm_mat4_identity(model);
  glm_rotate(model, TAU * 0.33, (vec3) { 0, 1, 0 });
  glm_mat4_mulv3(model, pnt_ligs[1], 1, pnt_ligs[1]);

  glm_mat4_identity(model);
  glm_rotate(model, TAU * 0.66, (vec3) { 0, 1, 0 });
  glm_mat4_mulv3(model, pnt_ligs[2], 1, pnt_ligs[2]);

  while (!glfwWindowShouldClose(canvas.window)) {
    // Draw lights
    glUseProgram(shader_lig);
    canvas_unim4(shader_lig, "PROJ", proj[0]);
    canvas_unim4(shader_lig, "VIEW", view[0]);
    
    glm_mat4_identity(model);
    glm_translate(model, pnt_ligs[0]);
    glm_scale(model, (vec3) { 0.3, 0.3, 0.3 });
    canvas_uni3f(shader_lig, "COL", 1, 0, 0);
    canvas_unim4(shader_lig, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glm_mat4_identity(model);
    glm_translate(model, pnt_ligs[1]);
    glm_scale(model, (vec3) { 0.3, 0.3, 0.3 });
    canvas_uni3f(shader_lig, "COL", 0, 1, 0);
    canvas_unim4(shader_lig, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glm_mat4_identity(model);
    glm_translate(model, pnt_ligs[2]);
    glm_scale(model, (vec3) { 0.3, 0.3, 0.3 });
    canvas_uni3f(shader_lig, "COL", 0, 0, 1);
    canvas_unim4(shader_lig, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    if (!toggle) {
      glm_mat4_identity(model);
      glm_rotate(model, PI / 100, (vec3) { 0, 1, 0 });
      glm_mat4_mulv3(model, pnt_ligs[0], 1, pnt_ligs[0]);
      glm_mat4_mulv3(model, pnt_ligs[1], 1, pnt_ligs[1]);
      glm_mat4_mulv3(model, pnt_ligs[2], 1, pnt_ligs[2]);
    }

    // Draw objects
    glUseProgram(shader_obj);
    canvas_unim4(shader_obj, "PROJ", proj[0]);
    canvas_unim4(shader_obj, "VIEW", view[0]);
    canvas_uni3f(shader_obj, "CAM", cam.pos[0], cam.pos[1], cam.pos[2]);
    
    canvas_uni3f(shader_obj, "DIR_LIGS[0].COL", 0.1, 0.1, 0.1);
    canvas_uni3f(shader_obj, "DIR_LIGS[0].DIR", -1, 0, 0);
    
    canvas_uni3f(shader_obj, "SPT_LIGS[0].POS", cam.pos[0], cam.pos[1], cam.pos[2]);
    canvas_uni3f(shader_obj, "SPT_LIGS[0].DIR", cam.dir[0], cam.dir[1], cam.dir[2]);
    
    canvas_uni3f(shader_obj, "PNT_LIGS[0].COL", 1, 0, 0);
    canvas_uni3f(shader_obj, "PNT_LIGS[0].POS", pnt_ligs[0][0], pnt_ligs[0][1], pnt_ligs[0][2]);
    canvas_uni1f(shader_obj, "PNT_LIGS[0].CON", 1);
    canvas_uni1f(shader_obj, "PNT_LIGS[0].LIN", 0.0035);
    canvas_uni1f(shader_obj, "PNT_LIGS[0].QUA", 0.04);

    canvas_uni3f(shader_obj, "PNT_LIGS[1].COL", 0, 1, 0);
    canvas_uni3f(shader_obj, "PNT_LIGS[1].POS", pnt_ligs[1][0], pnt_ligs[1][1], pnt_ligs[1][2]);
    canvas_uni1f(shader_obj, "PNT_LIGS[1].CON", 1);
    canvas_uni1f(shader_obj, "PNT_LIGS[1].LIN", 0.0035);
    canvas_uni1f(shader_obj, "PNT_LIGS[1].QUA", 0.04);
 
    canvas_uni3f(shader_obj, "PNT_LIGS[2].COL", 0, 0, 1);
    canvas_uni3f(shader_obj, "PNT_LIGS[2].POS", pnt_ligs[2][0], pnt_ligs[2][1], pnt_ligs[2][2]);
    canvas_uni1f(shader_obj, "PNT_LIGS[2].CON", 1);
    canvas_uni1f(shader_obj, "PNT_LIGS[2].LIN", 0.0035);
    canvas_uni1f(shader_obj, "PNT_LIGS[2].QUA", 0.04);

    // Floor
    canvas_use_texture(shader_obj, "MAT.S_DIF", 0);
    canvas_uni3f(shader_obj, "MAT.AMB", 1.00, 1.00, 1.00);

    glm_mat4_identity(model);
    glm_translate(model, (vec3) { 0, -16, 0 });
    glm_scale(model, (vec3) { 10, 10, 10 });
    canvas_unim4(shader_obj, "MODEL", model[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Pumpkins
    canvas_use_texture(shader_obj, "MAT.S_DIF", 1);
    canvas_uni3f(shader_obj, "MAT.AMB", 0.10, 0.10, 0.10);
    for (i8 i = 0; i < 80; i++) {
      glm_mat4_identity(model);
      glm_translate(model, poss[i]);
      glm_rotate(model, cos(sin(poss[i][2]) * tan(poss[i][0])), (vec3) { tan(poss[i][0]), sin(poss[i][2]), 0.5 });
      if (!i) glm_scale(model, (vec3) { 2, 2, 2 });
      if (!toggle) poss[i][1] = CIRCULAR_CLAMP(-10, poss[i][1] - 0.05, 10);

      canvas_unim4(shader_obj, "MODEL", model[0]);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

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
