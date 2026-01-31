#include "canvas.h"

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

  u32 shader = shader_create_program("obj");
  generate_proj_mat(&cam, shader);
  generate_view_mat(&cam, shader);
  canvas_set_pnt_lig(shader, (PntLig) { WHITE, { 2 }, 1, 0.07, 0.017 }, 0);
  u32 hud_shader = shader_create_program("hud");
  generate_ortho_mat(&cam, hud_shader);

  Font    font   = { texture( "font"), 20, 5, 7.0 / 5 };
  Entity* sphere = entity_create("sphere",               (Material) { DEEP_PURPLE, 0.3, 0.6 });
  Entity* glass  = entity_create("cube",                 (Material) { WHITE,       0.3, 0.6, .tex = texture("glass") });
  Entity* cube   = entity_create("cube",                 (Material) { DEEP_GREEN,  0.3, 0.6 });
  Entity* lamp   = entity_create("sphere",               (Material) { WHITE,                 .lig = 1 });
  Text3D* text   = text_3d_create("ALAANVV", font, 0.01, (Material) { DEEP_RED,    0.5, 0.5, .lig = 1 });

  play_audio("idk");

  while (!glfwWindowShouldClose(cam.window)) {
    VEC3_COPY(VEC3(sin(glfwGetTime())      * 8, 0,    cos(glfwGetTime()) * 8),      sphere->pos);
    VEC3_COPY(VEC3(sin(glfwGetTime() + PI) * 8, -0.5, cos(glfwGetTime() + PI) * 8), cube->pos);
    VEC3_COPY(VEC3(sin(glfwGetTime() + PI) * 5, -0.5, cos(glfwGetTime() + PI) * 5), glass->pos);
    VEC3_COPY(VEC3(glfwGetTime() * 5, 0, 0), text->rot);
    VEC3_COPY(VEC3(0, 0, -5), text->pos);

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
