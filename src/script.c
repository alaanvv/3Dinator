#include "canvas.h"
#include "meshes.h" 

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(x, y, z) (MAX(MIN(z, y), x))
#define PI  3.14159
#define PI2 PI / 2
#define PI4 PI / 4

#define WIDTH  1920
#define HEIGHT 1080
#define SPEED 0.1
#define SENSITIVITY 0.001
#define CAMERA_LOCK PI2 * 0.9
#define FOV PI4

void handle_inputs(GLFWwindow*);

Canvas canvas = { WIDTH, HEIGHT };
Camera cam    = { WIDTH, HEIGHT, FOV, 0.1, 100, 0, 0, { 4, 2, 4 }, { 0, 0, -1 }, { 1, 0, 0 }};

mat4 model, view, proj;
vec3 mouse;

u32 game[8][8] = {
  { 23, 25, 24, 22, 21, 24, 25, 23 }, // x1 = King
  { 26, 26, 26, 26, 26, 26, 26, 26 }, // x2 = Queen
  { 00, 00, 00, 00, 00, 00, 00, 00 }, // x3 = Rook
  { 00, 00, 00, 00, 00, 00, 00, 00 }, // x4 = Bishop
  { 00, 00, 00, 00, 00, 00, 00, 00 }, // x5 = Knight
  { 00, 00, 00, 00, 00, 00, 00 ,00 }, // x6 = Pawn
  { 16, 16, 16, 16, 16, 16, 16, 16 }, // 1x = White
  { 13, 15, 14, 12, 11, 14, 15, 13 }, // 2x = Black
};

i8 selected[2] = { -1, -1 };

// ---

void main() {
  canvas_init(&canvas, (CanvasInitConfig) { 1, "Chess" });

  Model* cube = model_create("obj/cube.obj");
  Model* piece_models[] = { 
    model_create("obj/king.obj"),
    model_create("obj/queen.obj"),
    model_create("obj/rook.obj"),
    model_create("obj/bishop.obj"),
    model_create("obj/knight.obj"),
    model_create("obj/pawn.obj")
  };

  Material black_piece = { { 0.20, 0.20, 0.20 }, 1, 1, 0, 255, 0, 0, 1 };
  Material white_piece = { { 0.70, 0.70, 0.70 }, 1, 1, 0, 255, 0, 0, 1 };
  Material selec_piece = { { 0.45, 0.30, 0.60 }, 1, 1, 0, 255, 0, 0, 1 };
  Material alert_piece = { { 0.95, 0.30, 0.30 }, 1, 1, 0, 255, 0, 0, 1 };
  Material board       = { { 0.60, 0.60, 0.60 }, 1, 1, 0, 255, 0, 0, 1 };
  Material board_top   = { { 1.00, 1.00, 1.00 }, 1, 1, 0, 255, 2, 0, 1 };

  u32 shader        = shader_create_program("shd/obj.v", "shd/obj.f");
  u32 buffer_shader = shader_create_program("shd/obj.v", "shd/buf.f");

  canvas_create_texture(GL_TEXTURE0, "img/w.ppm",      GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_NEAREST, GL_NEAREST);
  canvas_create_texture(GL_TEXTURE1, "img/b.ppm",      GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_NEAREST, GL_NEAREST);
  canvas_create_texture(GL_TEXTURE2, "img/board.ppm",  GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_NEAREST, GL_NEAREST);

  generate_proj_mat(cam, proj);
  generate_view_mat(cam, view);

  while (!glfwWindowShouldClose(canvas.window)) {
    glUseProgram(buffer_shader);
    canvas_unim4(buffer_shader, "PROJ", proj[0]);
    canvas_unim4(buffer_shader, "VIEW", view[0]);
    canvas_uni3f(buffer_shader, "CAM", cam.pos[0], cam.pos[1], cam.pos[2]);

    for (u8 row = 0; row < 8; row++) {
      for (u8 col = 0; col < 8; col++) {
        canvas_uni3f(buffer_shader, "COL", (f32) row / 10, (f32) col / 10, 0);

        glm_mat4_identity(cube->model_matrix);
        glm_translate(cube->model_matrix, (vec3) { row + 0.1, 0, col + 0.1 });
        glm_scale(cube->model_matrix, (vec3) { 0.8, 0.1, 0.8 });
        model_draw(cube, shader);

        if (game[row][col] % 100) {
          Model* piece = piece_models[game[row][col] % 10 - 1];
          glm_mat4_identity(piece->model_matrix);
          glm_translate(piece->model_matrix, (vec3) { row + 0.5, 0, col + 0.5 });
          model_draw(piece, shader);
        }
      }
    }

    handle_inputs(canvas.window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader);
    canvas_uni3f(shader, "PNT_LIGS[0].COL", 1, 1, 1);
    canvas_uni3f(shader, "PNT_LIGS[0].POS", 0, 8, 4);
    canvas_uni1f(shader, "PNT_LIGS[0].CON", 1);
    canvas_uni1f(shader, "PNT_LIGS[0].LIN", 0.07);
    canvas_uni1f(shader, "PNT_LIGS[0].QUA", 0.017);
    canvas_unim4(shader, "PROJ", proj[0]);
    canvas_unim4(shader, "VIEW", view[0]);
    canvas_uni3f(shader, "CAM", cam.pos[0], cam.pos[1], cam.pos[2]);

    // Board
    canvas_set_material(shader, board);
    glm_mat4_identity(cube->model_matrix);
    glm_scale(cube->model_matrix, (vec3) { 8, 1, 8 });
    glm_translate(cube->model_matrix, (vec3) { 0, -1, 0 });
    model_draw(cube, shader);

    canvas_set_material(shader, board_top);
    glm_mat4_identity(cube->model_matrix);
    glm_scale(cube->model_matrix, (vec3) { 8, 0.01, 8 });
    model_draw(cube, shader);

    for (u8 row = 0; row < 8; row++) {
      for (u8 col = 0; col < 8; col++) {
        if (!(game[row][col] % 100)) {
          if (game[row][col] / 100) {
            canvas_set_material(shader, selec_piece);

            glm_mat4_identity(cube->model_matrix);
            glm_translate(cube->model_matrix, (vec3) { row, 0, col });
            glm_scale(cube->model_matrix, (vec3) { 0.7, 0.1, 0.7 });
            glm_translate(cube->model_matrix, (vec3) { 0.2, 0, 0.2 });
            model_draw(cube, shader);
          }
          continue;
        }

        canvas_set_material(shader, game[row][col] % 100 < 20 ? white_piece : black_piece);
        if (selected[0] == row && selected[1] == col) 
          canvas_set_material(shader, selec_piece);
        if (game[row][col] / 100) 
          canvas_set_material(shader, alert_piece);

        glm_mat4_identity(cube->model_matrix);
        glm_translate(cube->model_matrix, (vec3) { row, 0, col });
        glm_scale(cube->model_matrix, (vec3) { 0.7, 0.1, 0.7 });
        glm_translate(cube->model_matrix, (vec3) { 0.2, 0, 0.2 });
        model_draw(cube, shader);

        Model* curr_piece = piece_models[game[row][col] % 10 - 1];
        glm_mat4_identity(curr_piece->model_matrix);
        glm_translate(curr_piece->model_matrix, (vec3) { row + 0.5, 0, col + 0.5 });
        model_draw(curr_piece, shader);
      }
    }

    glfwPollEvents();
    glfwSwapBuffers(canvas.window); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  glfwTerminate();
}

void calc_lateral_diagonal_moves(u8 row, u8 col, c8 mov, u8 max_dis) {
  u8 dir_flags[4] = { 1, 1, 1, 1 };
  u8 piece = game[row][col];
  u8 _row, _col;

  for (u8 dis = 1; dis <= max_dis; dis++) {
    for (u8 dir = 0; dir < 4; dir++) {
      if (!dir_flags[dir]) continue;

      _row = row + (mov == 'L' ? (dir == 0 ? -dis : dir == 1 ? dis : 0) : (dir == 0 || dir == 1 ? -dis : dir == 2 || dir == 3 ? dis : 0));
      _col = col + (mov == 'L' ? (dir == 2 ? -dis : dir == 3 ? dis : 0) : (dir == 0 || dir == 2 ? -dis : dir == 1 || dir == 3 ? dis : 0));

      if (_row < 0 || _row > 7 || _col < 0 || _col > 7 || game[_row][_col] / 10 == piece / 10) { dir_flags[dir] = 0; continue; }
      if (game[_row][_col] % 100 && game[_row][_col] % 100 / 10 != piece % 100 / 10) dir_flags[dir] = 0;
      game[_row][_col] += 100;
    }
  }
}

void calc_pawn_moves(u8 row, u8 col) {
  u8 piece = game[row][col];
  i8 dir = piece % 100 > 20 ? 1 : -1;

  if (col - 1 >= 0 && game[row + dir][col - 1] % 100 && game[row + dir][col - 1] % 100 / 10 != piece % 100 / 10) game[row + dir][col - 1] += 100;
  if (col + 1 <= 7 && game[row + dir][col + 1] % 100 && game[row + dir][col + 1] % 100 / 10 != piece % 100 / 10) game[row + dir][col + 1] += 100;

  if (game[row + dir][col] % 100) return;
  game[row + dir][col] += 100;

  if (row != (piece % 100 > 20 ? 1 : 6) || game[row + dir * 2][col] % 100) return;
  game[row + dir * 2][col] += 100;
}

void calc_knight_moves(u8 row, u8 col) {
  u8 piece = game[row][col];
  u8 _row, _col;

  for (u8 i = 1; i <= 2; i++) {
    for (i8 row_m = -1; row_m <= 1; row_m += 2) {
      for (i8 col_m = -1; col_m <= 1; col_m += 2) {
        _row = row + (i                * row_m);
        _col = col + ((i == 1 ? 2 : 1) * col_m);

        if (_row >= 0 && _row <= 7 && _col >= 0 && _col <= 7 && game[_row][_col] % 100 / 10 != piece % 100 / 10)
          game[_row][_col] += 100;
      }
    }
  }
}

void update_selection(u8 row, u8 col) {
  u8 is_movement = game[row][col] / 100;
  for (u8 row = 0; row < 8; row++)
    for (u8 col = 0; col < 8; col++)
      game[row][col] %= 100;

  if (is_movement) {
    u8 piece = game[selected[0]][selected[1]];
    game[selected[0]][selected[1]] = 0;
    if ((row == 0 || row == 7) && piece % 10 == 6) piece -= 4;
    game[row][col] = piece;

    selected[0] = -1;
    selected[1] = -1;
    return;
  }

  selected[0] = row;
  selected[1] = col;

  switch (game[row][col] % 10) {
    case 1:
      calc_lateral_diagonal_moves(row, col, 'L', 1);
      calc_lateral_diagonal_moves(row, col, 'D', 1);
      break;

    case 2:
      calc_lateral_diagonal_moves(row, col, 'L', 7);
      calc_lateral_diagonal_moves(row, col, 'D', 7);
      break;

    case 3:
      calc_lateral_diagonal_moves(row, col, 'L', 7);
      break;

    case 4:
      calc_lateral_diagonal_moves(row, col, 'D', 7);
      break;

    case 5:
      calc_knight_moves(row, col);
      break;

    case 6:
      calc_pawn_moves(row, col);
      break;
  }
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
  };

  // MISC
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) { cam.fov = MIN(cam.fov + PI / 100, FOV); generate_proj_mat(cam, proj); }
  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) { cam.fov = MAX(cam.fov - PI / 100, 0.1); generate_proj_mat(cam, proj); }
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);

  // MOUSE BUTTON
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) mouse[2] = 0;
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mouse[2]) {
    mouse[2] = 1;
    f32* buffer = malloc(sizeof(f32) * 3);
    glReadPixels(WIDTH / 2, HEIGHT / 2, 1, 1, GL_RGB, GL_FLOAT, buffer);
    update_selection(roundf(buffer[0] * 10), roundf(buffer[1] * 10));
  }

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
  mouse[0] = x;
  mouse[1] = y;
}
