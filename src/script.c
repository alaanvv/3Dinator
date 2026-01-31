#include "canvas.h"

u32 shader, hud_shader;

CanvasConfig config = { 
  .title = "FONT", 
  .capture_mouse = 1, 
  .fullscreen = 0, 
  .screen_size = 0.5, 
  .clear_color = BLACK 
};

Camera cam = { 
  .fov = PI4, 
  .near_plane = 0.01, 
  .far_plane = 100, 
  .sensitivity = 0.001, 
  .camera_lock = PI2 * 0.9,
  .speed = 3
};

// ---

int main() {
  canvas_init(&cam, config);

  // Material
  Material m_sphere = { DEEP_PURPLE, 0.3, 0.6 };
  Material m_cube   = { DEEP_GREEN,  0.3, 0.6 };
  Material m_glass  = { WHITE,       0.3, 0.6, .tex = texture("glass") };
  Material m_text   = { DEEP_RED,    0.5, 0.5, .lig = 1 };
  Material m_lamp   = { WHITE,                 .lig = 1 };

  // Light
  PntLig light = { WHITE, { 2 }, 1, 0.07, 0.017 };

  // Model
  Model* mo_sphere = model_create("sphere", m_sphere);
  Model* mo_glass  = model_create("cube",   m_glass);
  Model* mo_cube   = model_create("cube",   m_cube);
  Model* mo_lamp   = model_create("sphere", m_lamp);

  // Font
  Font font = { texture( "font"), 20, 5, 7.0 / 5 };

  // Audio
  play_audio("idk");

  // Shader
  shader = shader_create_program("obj");
  generate_proj_mat(&cam, shader);
  generate_view_mat(&cam, shader);
  canvas_set_pnt_lig(shader, light, 0);
  hud_shader = shader_create_program("hud");
  generate_ortho_mat(&cam, hud_shader);

  // Entities
  Entity* sphere = entity_create(mo_sphere);
  Entity* glass  = entity_create(mo_cube);
  Entity* cube   = entity_create(mo_glass);
  Entity* lamp   = entity_create(mo_lamp);
  Text3D* text   = text_3d_create("ALAANVV", font, 0.01, m_text);

  while (!glfwWindowShouldClose(cam.window)) {
    VEC3_COPY(VEC3(sin(glfwGetTime())      * 8, 0,    cos(glfwGetTime()) * 8),      sphere->pos);
    VEC3_COPY(VEC3(sin(glfwGetTime() + PI) * 8, -0.5, cos(glfwGetTime() + PI) * 8), cube->pos);
    VEC3_COPY(VEC3(sin(glfwGetTime() + PI) * 5, -0.5, cos(glfwGetTime() + PI) * 5), glass->pos);
    VEC3_COPY(VEC3(0, 0, -5), text->pos);
    VEC3_COPY(VEC3(glfwGetTime() * PI4, sin(glfwGetTime()* 2) * PI4, sin(glfwGetTime()*7) * PI2), text->rot);

    canvas_draw_3d_entities(shader);

    // HUD Drawing
    glUseProgram(hud_shader);
    hud_draw_rec(hud_shader, texture("hand"), (vec3) WHITE, 0, 0, 300, 400);
    c8 buffer[10];
    sprintf(buffer, "%d FPS", (i32) cam.fps);
    hud_draw_text(hud_shader, buffer, 10, cam.height - font.size * font.ratio - 10, font, (vec3) WHITE);
    hud_draw_text(hud_shader, "hello world", 500, 20, font, (vec3) WHITE);

    // Finish
    glfwSwapBuffers(cam.window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera_handle_inputs(&cam, shader);
    glfwPollEvents();
    update_fps(&cam);
  }

  glfwTerminate();
  return 0;
}
