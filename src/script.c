#include "canvas.h"

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, y, z) (MAX(MIN(z, y), x))

#define PI  3.14159
#define TAU PI * 2
#define PI2 PI / 2
#define PI4 PI / 4

#define WIDTH  1920
#define HEIGHT 1080
#define SPEED 0.1 
#define UPSCALE 0.2
#define SENSITIVITY 0.001
#define CAMERA_LOCK PI2 * 0.99
#define FOV PI4

void handle_inputs(GLFWwindow*);

// ---

Camera cam    = { WIDTH, HEIGHT, FOV, 0.1, 100, 0, 0, { 0, 0, 0 }, { 0, 0, -1 }, { 1, 0, 0 }};
Canvas canvas = { WIDTH, HEIGHT };
mat4 view, proj, blank;
vec3 mouse;
u32 shader;

Material m_light = { { 1.00, 1.00, 1.00 }, 0.0, 0.0, 0.0, 000, 0, 0, 0, 1 };
Material m_cube  = { { 1.00, 1.00, 1.00 }, 0.5, 0.5, 0.5, 255, 0, 0, 1, 0 };

PntLig light = { { 1, 1, 1 }, { 0.5, 0.5, 0.5 }, 1, 0.07, 0.017 };

// ---

void main() {
  canvas_init(&canvas, (CanvasInitConfig) { 1, "Room" });
  glm_mat4_identity(blank);

  Model* cube = model_create("obj/cube.obj");

  u32 lowres_fbo = canvas_create_FBO(WIDTH * UPSCALE, HEIGHT * UPSCALE, GL_NEAREST, GL_NEAREST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  generate_proj_mat(cam, proj);
  generate_view_mat(cam, view);

  canvas_create_texture(GL_TEXTURE0, "img/w.ppm", TEXTURE_DEFAULT);
  canvas_create_texture(GL_TEXTURE1, "img/b.ppm", TEXTURE_DEFAULT);

  shader = shader_create_program("shd/obj.v", "shd/obj.f");
  
  canvas_set_pnt_lig(shader, light, 0);
  canvas_unim4(shader, "PROJ", proj[0]);
  canvas_unim4(shader, "VIEW", view[0]);

  f32 acc = 0;
  while (!glfwWindowShouldClose(canvas.window)) {
    canvas_set_material(shader, m_light);
    glm_mat4_identity(cube->model);
    model_draw(cube, shader);

    canvas_set_material(shader, m_cube);
    glm_mat4_identity(cube->model);
    glm_translate(cube->model, (vec3) { sin(acc) * 5, 0, cos(acc) * 5 });
    model_draw(cube, shader);

    glBlitNamedFramebuffer(0, lowres_fbo, 0, 0, WIDTH, HEIGHT, 0, 0, WIDTH * UPSCALE, HEIGHT * UPSCALE, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitNamedFramebuffer(lowres_fbo, 0, 0, 0, WIDTH * UPSCALE, HEIGHT * UPSCALE, 0, 0, WIDTH, HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glfwPollEvents();
    handle_inputs(canvas.window);
    glfwSwapBuffers(canvas.window); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    acc += 0.01;
  }
  glfwTerminate();
}

void handle_inputs(GLFWwindow* window) {
  // MOVEMENT
  vec3 prompted_move = { 
    (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ?  SPEED : 0) + (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? -SPEED : 0), 
    (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ?  SPEED : 0) + (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ? -SPEED : 0), 
    (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ?  SPEED : 0) + (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? -SPEED : 0)
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
    generate_view_mat(cam, view);  
    canvas_unim4(shader, "VIEW", view[0]);
    canvas_uni3f(shader, "CAM", cam.pos[0], cam.pos[1], cam.pos[2]);
  };

  // MISC
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) { cam.fov = MIN(cam.fov + PI / 100, FOV); generate_proj_mat(cam, proj); canvas_unim4(shader, "PROJ", proj[0]); }
  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) { cam.fov = MAX(cam.fov - PI / 100, 0.1); generate_proj_mat(cam, proj); canvas_unim4(shader, "PROJ", proj[0]); }
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);

  // MOUSE MOVEMENT
  f64 x, y;
  glfwGetCursorPos(window, &x, &y);

  if (!mouse[0]) {
    mouse[0] = x;
    mouse[1] = y;
  }
  if (x == mouse[0] && y == mouse[1]) return;

  cam.yaw  += (x - mouse[0]) * SENSITIVITY;
  cam.pitch = CLAMP(-CAMERA_LOCK, cam.pitch + (mouse[1] - y) * SENSITIVITY, CAMERA_LOCK);

  cam.dir[0] = cos(cam.yaw - PI2) * cos(cam.pitch);
  cam.dir[1] = sin(cam.pitch);
  cam.dir[2] = sin(cam.yaw - PI2) * cos(cam.pitch);
  cam.rig[0] = cos(cam.yaw) * cos(cam.pitch);
  cam.rig[2] = sin(cam.yaw) * cos(cam.pitch);
  glm_normalize(cam.rig);

  generate_view_mat(cam, view);
  canvas_unim4(shader, "VIEW", view[0]);
  mouse[0] = x;
  mouse[1] = y;
}
