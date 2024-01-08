#include "canvas.h"

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, y, z) (MAX(MIN(z, y), x))
#define UNI(shader, name) (glGetUniformLocation(shader, name))
#define PI  3.14159
#define PI4 PI / 4

#define VERTEX_SHADER   "shd/vertex.glsl"
#define FRAGMENT_SHADER "shd/fragment.glsl"
#define TEXTURE_0 "img/pmk.ppm"
#define WIDTH  800
#define HEIGHT 800
#define MOVEMENT_SPEED 0.1
#define CAMERA_SPEED 0.05
#define CAMERA_LOCK PI4
#define OBJ cube

typedef uint8_t  u8;
typedef uint32_t u32;
typedef int8_t   i8;
typedef float    f32;

void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
void handle_inputs(GLFWwindow* window);

// --- Setup

#include "mesh.h" 

Canvas canvas = { NULL, WIDTH, HEIGHT };
Camera cam = { WIDTH, HEIGHT, PI4, 0.1, 100, { 0, 0, 10 }, { 0, 0, -1 }, { 1, 0, 0 } };

mat4 view, proj;
f32  last_frame, fps;

// --- Main

i8 main() {
  canvas_init(&canvas, "Light");
  glfwSetKeyCallback(canvas.window, key_callback);

  u32 VBO = canvas_create_VBO();
  u32 VAO = canvas_create_VAO();

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*) 0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*) (sizeof(f32) * 3));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBufferData(GL_ARRAY_BUFFER, sizeof(OBJ), OBJ, GL_STATIC_DRAW);

  u32 shader_program = shader_create_program(VERTEX_SHADER, FRAGMENT_SHADER);

  canvas_config_texture(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_NEAREST, GL_NEAREST);
  canvas_create_texture(GL_TEXTURE0, TEXTURE_0, shader_program, "TEXTURE_0", 0);

  mat4 model;
  generate_proj_mat(cam, proj);
  generate_view_mat(cam, view);

  while (!glfwWindowShouldClose(canvas.window)) {
    glm_mat4_identity(model);
    glm_rotate(model, glfwGetTime(), (vec3) { 0.5, 0.2, 0 });
    glUniformMatrix4fv(UNI(shader_program, "MODEL"), 1, GL_FALSE, (const f32*) { model[0] });
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glfwSwapBuffers(canvas.window); 
    glfwPollEvents();
    handle_inputs(canvas.window);
    canvas_clear((RGBA) { 0.8, 0.7, 0.7, 1 });

    glUniformMatrix4fv(UNI(shader_program, "PROJ"),  1, GL_FALSE, (const f32*) { proj[0] });
    glUniformMatrix4fv(UNI(shader_program, "VIEW"),  1, GL_FALSE, (const f32*) { view[0] });
  }

  glfwTerminate();
  return 0;
}

void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
  if (action != GLFW_PRESS) return;

  switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, 1); 
      break;
  }
}

void handle_inputs(GLFWwindow* window) {
  u8 should_reload_view, should_reload_proj;
  inline void rv() { should_reload_view = 1; } 
  inline void rp() { should_reload_proj = 1; }
  inline void rotate_cam(vec3 dir) {
    if (dir[1]) {
      mat4 rot;
      glm_mat4_identity(rot);
      glm_rotate(rot, CAMERA_SPEED, (vec3) { 0, dir[1], 0 });
      glm_mat4_mulv3(rot, cam.dir, 1, cam.dir);
      glm_mat4_mulv3(rot, cam.rig, 1, cam.rig);
    }
    if (dir[0]) {
      f32 ang = CLAMP(-CAMERA_LOCK, cam.dir[1] + CAMERA_SPEED * dir[0], CAMERA_LOCK);

      cam.dir[1] = ang;
    }

    rv();
  }
  inline void move_cam(vec3 dir) {
    vec3 move;
    if (dir[0]) {
      glm_vec3_scale(cam.rig, dir[0], move);
      glm_vec3_add(cam.pos, move, cam.pos);
    }
    else if (dir[2]) {
      glm_vec3_scale(cam.dir, dir[2], move);
      glm_vec3_add(cam.pos, move, cam.pos);
    }
    else cam.pos[1] += dir[1];

    rv();
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move_cam((vec3) {  0,  0,  MOVEMENT_SPEED });
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move_cam((vec3) {  0,  0, -MOVEMENT_SPEED });
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move_cam((vec3) {  MOVEMENT_SPEED,  0,  0 });
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move_cam((vec3) { -MOVEMENT_SPEED,  0,  0 });
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) move_cam((vec3) {  0,  MOVEMENT_SPEED,  0 });
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) move_cam((vec3) {  0, -MOVEMENT_SPEED,  0 });

  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) rotate_cam((vec3) {  1,  0 });
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) rotate_cam((vec3) { -1,  0 });
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) rotate_cam((vec3) {  0,  1 });
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) rotate_cam((vec3) {  0, -1 }); 

  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) { cam.fov += PI / 100; rp(); }
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) { cam.fov -= PI / 100; rp(); }

  if (should_reload_view) generate_view_mat(cam, view);
  if (should_reload_proj) generate_proj_mat(cam, proj);
}
