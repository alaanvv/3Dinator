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
  canvas_config_texture(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_NEAREST, GL_NEAREST);
  glfwSetInputMode(canvas.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(canvas.window, mouse_callback);
  glfwSetKeyCallback(canvas.window, key_callback);

  u32 VAO_cube = canvas_create_VAO();
  u32 VBO_cube = canvas_create_VBO();
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (3 * sizeof(f32)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) (6 * sizeof(f32)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  u32 VAO_light = canvas_create_VAO();
  u32 VBO_light = VBO_cube;
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void*) 0);
  glEnableVertexAttribArray(0);

  shader_lig = shader_create_program("shd/obj.v", "shd/light.f");
  shader     = shader_create_program("shd/obj.v", "shd/obj.f");
  glUseProgram(shader_lig);
  glUniform3f(UNI(shader_lig, "C_LIG"), c_lig[0], c_lig[1], c_lig[2]);
  glUseProgram(shader);
  glUniform3f(UNI(shader, "MAT.amb"), 0.29225,  0.29225,  0.29225);
  glUniform3f(UNI(shader, "MAT.dif"), 0.70754,  0.70754,  0.70754);
  glUniform3f(UNI(shader, "MAT.spc"), 0.508273, 0.508273, 0.508273);
  glUniform1f(UNI(shader, "MAT.shi"), 51.2);
  glUniform3f(UNI(shader, "LIG.amb"), c_lig[0], c_lig[1], c_lig[2]);
  glUniform3f(UNI(shader, "LIG.dif"), c_lig[0], c_lig[1], c_lig[2]);
  glUniform3f(UNI(shader, "LIG.spc"), c_lig[0], c_lig[1], c_lig[2]);
  glUniform3f(UNI(shader, "LIG.pos"), 0, 0, 0);

  canvas_create_texture(GL_TEXTURE0, "img/pmk.ppm", shader, "MAT.s_dif", 0);
  canvas_create_texture(GL_TEXTURE1, "img/spc.ppm", shader, "MAT.s_spc", 1);

  generate_proj_mat(cam, proj);
  generate_view_mat(cam, view);

vec3 poss[30] = {
    {-6.00, 13.00, -9.00},
    {5.00, 6.00, 10.00},
    {-2.00, 11.00, 4.00},
    {-9.00, 14.00, -6.00},
    {1.00, 7.00, -5.00},
    {0.00, 18.00, -3.00},
    {-3.00, 5.00, 1.00},
    {8.00, 14.00, 1.00},
    {-2.00, 0.00, 6.00},
    {0.00, 10.00, 6.00},
    {-4.00, 1.00, -5.00},
    {6.00, 2.00, 5.00},
    {-9.00, 4.00, 8.00},
    {-2.00, 12.00, -8.00},
    {8.00, 11.00, 4.00},
    {6.00, 15.00, -5.00},
    {0.00, 1.00, -7.00},
    {-10.00, 16.00, -3.00},
    {-8.00, 7.00, 2.00},
    {5.00, 7.00, 8.00},
    {2.00, 13.00, 8.00},
    {9.00, 6.00, -7.00},
    {-1.00, 2.00, -8.00},
    {4.00, 7.00, 4.00},
    {-3.00, 1.00, 8.00},
    {-2.00, 3.00, -3.00},
    {5.00, 8.00, -9.00},
    {9.00, 0.00, -1.00},
    {-7.00, 7.00, -7.00},
    {7.00, 2.00, 0.00}
};

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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO_cube);
    glBindVertexArray(VAO_cube);
    for (i8 i = 0; i < 30; i++) {
      glm_mat4_identity(model);
      glm_translate(model, poss[i]);
      glm_rotate(model, cos(sin(poss[i][2]) * tan(poss[i][0])), (vec3) { tan(poss[i][0]), sin(poss[i][2]), 0.5 });
      glUniformMatrix4fv(UNI(shader, "MODEL"), 1, GL_FALSE, (const f32*) { model[0] });
      glUniform3f(UNI(shader, "MAT.col"), .1, .1, .1);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      
      if (!toggle) {
        poss[i][1] -= 0.05;
        if (poss[i][1] < -10) poss[i][1] = 10;
      }
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
