#include "canvas.h"

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, y, z) (MAX(MIN(z, y), x))
#define UNI(shd, uni) (glGetUniformLocation(shd, uni))

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
u32 shader, shader_lig;

vec3 c_lig = { 1, 1, 1 };

u8 toggle;

// --- Main

i8 main() {
  canvas_init(&canvas, "Light");
  glfwSetInputMode(canvas.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(canvas.window, mouse_callback);
  glfwSetKeyCallback(canvas.window, key_callback);

  u32 VAO_cube = canvas_create_VAO();
  u32 VBO_cube = canvas_create_VBO();
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*) 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*) (3 * sizeof(f32)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  u32 VAO_light = canvas_create_VAO();
  u32 VBO_light = VBO_cube;
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*) 0);
  glEnableVertexAttribArray(0);

  shader_lig = shader_create_program("shd/v.glsl", "shd/f-light.glsl");
  shader     = shader_create_program("shd/v.glsl", "shd/f.glsl");
  glUseProgram(shader_lig);
  glUniform3f(UNI(shader_lig, "C_LIG"), c_lig[0], c_lig[1], c_lig[2]);
  glUseProgram(shader);
  glUniform3f(UNI(shader, "C_LIG"), c_lig[0], c_lig[1], c_lig[2]);
  glUniform3f(UNI(shader, "P_LIG"), 0, 0, 0);

  generate_proj_mat(cam, proj);
  generate_view_mat(cam, view);

  inline f32 r_rot() { return (random() % (i16) TAU * 1e2) / 1e2; }
  f32 t_me = r_rot();
  f32 t_ve = r_rot();
  f32 t_ea = r_rot();
  f32 t_ma = r_rot();
  f32 t_ju = r_rot();
  f32 t_sa = r_rot();
  f32 t_ur = r_rot();
  f32 t_ne = r_rot();

  while (!glfwWindowShouldClose(canvas.window)) {
    // Sun
    glm_mat4_identity(model);
    glUseProgram(shader_lig);
    glUniformMatrix4fv(UNI(shader_lig, "MODEL"), 1, GL_FALSE, (const f32*) { model[0] });
    glUniformMatrix4fv(UNI(shader_lig, "PROJ"),  1, GL_FALSE, (const f32*) { proj[0] });
    glUniformMatrix4fv(UNI(shader_lig, "VIEW"),  1, GL_FALSE, (const f32*) { view[0] });

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO_light);
    glBindVertexArray(VAO_light);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Change program
    glUseProgram(shader);
    glUniformMatrix4fv(UNI(shader, "PROJ"),  1, GL_FALSE, (const f32*) { proj[0] });
    glUniformMatrix4fv(UNI(shader, "VIEW"),  1, GL_FALSE, (const f32*) { view[0] });
    glUniform3f(UNI(shader, "P_CAM"), cam.pos[0], cam.pos[1], cam.pos[2]);

    void draw_planet(f32 r, f32 g, f32 b, f32 translation, f32 distance, f32 size) {
      glm_mat4_identity(model);
      glm_rotate(model, translation, (vec3) { 0, 1, 0 });
      glm_translate(model, (vec3) { distance });
      glm_scale(model, (vec3) { size, size, size });

      glUniformMatrix4fv(UNI(shader, "MODEL"), 1, GL_FALSE, (const f32*) { model[0] });
      glUniform3f(UNI(shader, "C_OBJ"), r, g, b);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO_cube);
      glBindVertexArray(VAO_cube);
      glDrawArrays(GL_TRIANGLES, 0, 36);   
    }

    draw_planet(0.58, 0.57, 0.60, t_me, 1, 0.05);
    draw_planet(0.57, 0.29, 0.06, t_ve, 1.75, 0.1);
    draw_planet(0.32, 0.39, 0.25, t_ea, 2.75, 0.09);
    draw_planet(0.93, 0.50, 0.40, t_ma, 3.75, 0.07);
    draw_planet(0.81, 0.71, 0.60, t_ju, 7, 0.45);
    draw_planet(0.68, 0.58, 0.43, t_sa, 9, 0.3);
    draw_planet(0.28, 0.60, 0.67, t_ur, 10.75, 0.253);
    draw_planet(0.27, 0.44, 1.00, t_ne, 16.5, 0.25);
    
    if (!toggle) { 
      t_me += TAU / 88;
      t_ve -= TAU / 225;
      t_ea += TAU / 365;
      t_ma += TAU / 687;
      t_ju += TAU / 4333;
      t_sa += TAU / 10747;
      t_ur -= TAU / 30589;
      t_ne += TAU / 60190;
    }

    glfwSwapBuffers(canvas.window); 
    glfwPollEvents();
    handle_inputs(canvas.window);
    canvas_clear((RGBA) { 0.00, 0.00, 0.07, 1 });
  }

  glfwTerminate();
  return 0;
}

// --- Function

void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
  if (action != GLFW_PRESS) return;

  if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, 1); 
  if (key == GLFW_KEY_F) toggle = 1 - toggle;
}

void handle_inputs(GLFWwindow* window) {
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
    generate_view_mat(cam, view);
  }

  // Movement
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move_cam((vec3) {  SPEED,  0,  0 });
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move_cam((vec3) { -SPEED,  0,  0 });
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) move_cam((vec3) {  0,  SPEED,  0 });
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) move_cam((vec3) {  0, -SPEED,  0 });
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move_cam((vec3) {  0,  0,  SPEED });
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move_cam((vec3) {  0,  0, -SPEED });

  // FOV
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
  glm_normalize(cam.dir);

  cam.rig[0] = cos(cam.yaw) * cos(cam.pitch);
  cam.rig[2] = sin(cam.yaw) * cos(cam.pitch);
  glm_normalize(cam.rig);

  generate_view_mat(cam, view);
}
