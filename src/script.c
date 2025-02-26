// TODO Font Rendering

#include "canvas.h"

#define SCREEN_SIZE 0.5
#define FULLSCREEN 0
#define SPEED 3
#define UPSCALE 0.2
#define SENSITIVITY 0.001
#define CAMERA_LOCK PI2 * 0.99
#define FOV PI4

void handle_inputs(GLFWwindow*);

// ---

Camera cam = { FOV, 0.01, 100 };
u32  shader, hud_shader;
vec3 mouse;
f32  fps;

Material m_sphere = { DEEP_PURPLE, 0.3, 0.6 };
Material m_cube   = { DEEP_ORANGE, 0.3, 0.6 };
Material m_glass  = { WHITE,       0.3, 0.6, .tex = GL_TEXTURE0 };

PntLig light = { WHITE, { 2 }, 1, 0.07, 0.017 };

// ---

int main() {
  canvas_init(&cam, (CanvasInitConfig) { "Room", 1, FULLSCREEN, SCREEN_SIZE });

  // Model
  Model* sphere = model_create("obj/sphere.obj", &m_sphere, 300);
  Model* glass  = model_create("obj/cube.obj",   &m_glass,  1);
  Model* cube   = model_create("obj/cube.obj",   &m_cube,   1);

  // Texture
  canvas_create_texture(GL_TEXTURE0, "img/glass.ppm", TEXTURE_DEFAULT);
  canvas_create_texture(GL_TEXTURE1, "img/hand.ppm",  TEXTURE_DEFAULT);

  // Shader
  shader = shader_create_program("shd/obj.v", "shd/obj.f");
  generate_proj_mat(&cam, shader);
  generate_view_mat(&cam, shader);
  canvas_set_pnt_lig(shader, light, 0);

  hud_shader = shader_create_program("shd/hud.v", "shd/hud.f");
  generate_ortho_mat(&cam, hud_shader);

  // FBO
  u32 lowres_fbo = canvas_create_FBO(cam.width * UPSCALE, cam.height * UPSCALE, GL_NEAREST, GL_NEAREST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  while (!glfwWindowShouldClose(cam.window)) {
    update_fps(&fps);

    model_bind(sphere, shader);
    model_draw_pnt_light(sphere, light, shader);

    model_bind(sphere, shader);
    glm_translate(sphere->model, VEC3(sin(glfwGetTime()) * 8, 0, cos(glfwGetTime()) * 8));
    model_draw(sphere, shader);

    model_bind(cube, shader);
    glm_translate(cube->model, VEC3(sin(glfwGetTime() + PI) * 8, -0.5, cos(glfwGetTime() + PI) * 8));
    model_draw(cube, shader);

    model_bind(glass, shader);
    glm_translate(glass->model, VEC3(sin(glfwGetTime() + PI) * 5, -0.5, cos(glfwGetTime() + PI) * 5));
    model_draw(glass, shader);

    glUseProgram(hud_shader);

    hud_draw_rec(hud_shader, GL_TEXTURE1, (vec3) WHITE, 0, 0, 300, 300);

    glUseProgram(shader);

    glBlitNamedFramebuffer(0, lowres_fbo, 0, 0, cam.width, cam.height, 0, 0, cam.width * UPSCALE, cam.height * UPSCALE, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitNamedFramebuffer(lowres_fbo, 0, 0, 0, cam.width * UPSCALE, cam.height * UPSCALE, 0, 0, cam.width, cam.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glfwSwapBuffers(cam.window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwPollEvents();
    handle_inputs(cam.window);
  }

  glfwTerminate();
  return 0;
}

void handle_inputs(GLFWwindow* window) {
  vec3 prompted_move = {
    (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? SPEED / fps : 0) + (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? -SPEED / fps : 0),
    (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ? SPEED / fps : 0) + (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ? -SPEED / fps : 0),
    (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? SPEED / fps : 0) + (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? -SPEED / fps : 0)
  };

  if (prompted_move[0] || prompted_move[1] || prompted_move[2]) {
    vec3 lateral  = { 0, 0, 0 };
    glm_vec3_scale(cam.rig, prompted_move[0], lateral);
    vec3 frontal  = { 0, 0, 0 };
    glm_vec3_scale(cam.dir, prompted_move[2], frontal);
    vec3 vertical = { 0, prompted_move[1], 0 };

    glm_vec3_add(cam.pos, lateral,  cam.pos);
    glm_vec3_add(cam.pos, frontal,  cam.pos);
    glm_vec3_add(cam.pos, vertical, cam.pos);
    generate_view_mat(&cam, shader);
  };

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);

  f64 x, y;
  glfwGetCursorPos(window, &x, &y);

  if (!mouse[0]) {
    mouse[0] = x;
    mouse[1] = y;
  }
  if (x == mouse[0] && y == mouse[1]) return;

  cam.yaw  += (x - mouse[0]) * SENSITIVITY;
  cam.pitch = CLAMP(-CAMERA_LOCK, cam.pitch + (mouse[1] - y) * SENSITIVITY, CAMERA_LOCK);

  glm_vec3_copy((vec3) { cos(cam.yaw - PI2) * cos(cam.pitch), sin(cam.pitch), sin(cam.yaw - PI2) * cos(cam.pitch) }, cam.dir);
  glm_vec3_copy((vec3) { cos(cam.yaw) * cos(cam.pitch), 0, sin(cam.yaw) * cos(cam.pitch) }, cam.rig);
  glm_normalize(cam.rig);

  generate_view_mat(&cam, shader);
  mouse[0] = x;
  mouse[1] = y;
}
