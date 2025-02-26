// TODO HUD
// TODO Font Rendering
// TODO Merge Light Materials into Light

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

Material m_light  = { { 1.0, 0.5, 0.5 }, 0.0, 0.0, 0, 0, 1 };
Material m_sphere = { { 1.0, 1.0, 1.0 }, 0.1, 0.5, 0, 1, 0 };
Material m_cube   = { { 1.0, 1.0, 1.0 }, 0.1, 0.5, 0, 1, 0 };
Material m_glass  = { { 1.0, 1.0, 1.0 }, 0.1, 0.5, 2, 1, 0 };

PntLig light = { { 1, 0.5, 0.5 }, { 0.5, 0.5, 0.5 }, 1, 0.07, 0.017 };

// ---

int main() {
  canvas_init(&cam, (CanvasInitConfig) { "Room", 1, FULLSCREEN, SCREEN_SIZE });

  Model* sphere = model_create("obj/sphere.obj", &m_sphere, 100);
  Model* glass  = model_create("obj/cube.obj",   &m_glass, 1);
  Model* cube   = model_create("obj/cube.obj",   &m_cube, 1);
  Model* plane  = model_create("obj/plane.obj",  &m_cube, 1);

  u32 lowres_fbo = canvas_create_FBO(cam.width * UPSCALE, cam.height * UPSCALE, GL_NEAREST, GL_NEAREST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  canvas_create_texture(GL_TEXTURE3, "img/hand.ppm",  TEXTURE_DEFAULT);
  canvas_create_texture(GL_TEXTURE2, "img/glass.ppm", TEXTURE_DEFAULT);

  shader     = shader_create_program("shd/obj.v", "shd/obj.f");
  generate_proj_mat(&cam, shader);
  generate_view_mat(&cam, shader);
  canvas_set_pnt_lig(shader, light, 0);

  hud_shader = shader_create_program("shd/hud.v", "shd/hud.f");
  generate_ortho_mat(&cam, hud_shader);

  while (!glfwWindowShouldClose(cam.window)) {
    update_fps(&fps);

    model_bind(sphere, shader);
    canvas_set_material(shader, m_light);
    model_draw(sphere, shader);

    model_bind(cube, shader);
    glm_translate(cube->model, (vec3) { sin(glfwGetTime() + PI) * 8, -0.5, cos(glfwGetTime() + PI) * 8 });
    model_draw(cube, shader);

    model_bind(glass, shader);
    glm_translate(glass->model, (vec3) { sin(glfwGetTime() + PI) * 5, -0.5, cos(glfwGetTime() + PI) * 5 });
    model_draw(glass, shader);

    glUseProgram(hud_shader);

    canvas_uni1i(hud_shader, "S_TEX", 3);
    model_bind(plane, hud_shader);
    glm_scale(plane->model, (vec3) {300, 300, 1.0});
    model_draw(plane, hud_shader);

    glUseProgram(shader);

    glBlitNamedFramebuffer(0, lowres_fbo, 0, 0, cam.width, cam.height, 0, 0, cam.width * UPSCALE, cam.height * UPSCALE, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitNamedFramebuffer(lowres_fbo, 0, 0, 0, cam.width * UPSCALE, cam.height * UPSCALE, 0, 0, cam.width, cam.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glfwPollEvents();
    handle_inputs(cam.window);
    glfwSwapBuffers(cam.window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
