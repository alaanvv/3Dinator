#include <time.h>
#include "render.h"
#include "mesh.h"

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define LEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define PI 3.1415962
#define TAU PI * 2
#define PI2 PI * 0.5
#define PI4 PI * 0.25

#define OBJ bottle 
#define WIDTH 200
#define HEIGHT 200
#define SCALE_X 5
#define SCALE_Y 5
#define CAMERA_Y_LOCK PI2
#define SPEED 0.1
#define CAMERA_SPEED PI4 * 1e-1

typedef float    f32;
typedef double   f64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   i8;
typedef int16_t  i16;

void handle_key(SDL_Event ev);

Canvas canvas = { NULL, NULL, WIDTH, HEIGHT, SCALE_X, SCALE_Y };
View view =     { 0.1, 1000, PI2, WIDTH / HEIGHT };
Camera camera;

// ---

u8 main() {
  render_proj_vec(&view);
  canvas_init(&canvas);

  Tri tri, tri_proj;
  Vec3 normal;
  f32 ry;

  SDL_Event ev;
  while (1) {
    while(SDL_PollEvent(&ev)) { 
      if (ev.type == SDL_QUIT) return 0; 
      else if (ev.type == SDL_KEYDOWN) handle_key(ev);
    }

    canvas_update(canvas, 5e1);

    ry += TAU / 1e2;

    for (u16 i = 0; i < LEN(OBJ); i++) {
      for (u8 p = 0; p < 3; p++) 
        for (u8 c = 0; c < 3; c++)
          tri[p][c] = OBJ[i][p][c];

      render_rotate(tri, (Vec3) { PI4, ry, 0 });
      render_translate(tri, (Vec3) { 0, 0, 9 });

      // Move world based on camera position
      render_translate(tri, (Vec3) { -camera.pos[0], -camera.pos[1], -camera.pos[2] });
      render_rotate(tri, (Vec3) { 0, 0, -camera.ang[1] * sin(camera.ang[0]) });
      render_rotate(tri, (Vec3) { -camera.ang[1] * cos(camera.ang[0]), -camera.ang[0], 0 });

      render_create_normal(tri, normal);
      if(normal[0] * tri[0][0] + normal[1] * tri[0][1] + normal[2] * tri[0][2] > 0.) continue;

      u8 light = 255 / 2 * ((normal[0] + 1));

      render_project(view, tri, tri_proj, canvas.width, canvas.height);
      canvas_color(canvas, (Color) { light, light, light, 255 });
      canvas_fill_triangle(canvas, tri_proj[0], tri_proj[1], tri_proj[2]); 
    }
  }

  return 0;
}

void handle_key(SDL_Event ev) {
  switch (ev.key.keysym.sym) {
    case 97: // A
      camera.pos[0] -= cos(camera.ang[0]) * SPEED;
      camera.pos[2] -= sin(camera.ang[0]) * SPEED;
      break;
    case 100: // D
      camera.pos[0] += cos(camera.ang[0]) * SPEED;
      camera.pos[2] += sin(camera.ang[0]) * SPEED;
      break;
    case 119: // W
      camera.pos[0] -= sin(camera.ang[0]) * SPEED;
      camera.pos[2] += cos(camera.ang[0]) * SPEED;
      break;
    case 115: // S
      camera.pos[0] += sin(camera.ang[0]) * SPEED;
      camera.pos[2] -= cos(camera.ang[0]) * SPEED;
      break;
    case 101: // E
      camera.pos[1] += SPEED;
      break;
    case 113: // Q
      camera.pos[1] -= SPEED;
      break;
    case 1073741904: // LEFT
      camera.ang[0] += CAMERA_SPEED;
      break;
    case 1073741903: // RIGHT
      camera.ang[0] -= CAMERA_SPEED;
      break;
    case 1073741906: // UP
      camera.ang[1] = MIN(camera.ang[1] + CAMERA_SPEED, CAMERA_Y_LOCK);
      break;
    case 1073741905: // DOWN
      camera.ang[1] = MAX(camera.ang[1] - CAMERA_SPEED, -CAMERA_Y_LOCK);
      break;
    case 114: // R
      camera.ang[0] = 0;
      camera.ang[1] = 0;
      break;
    case 102: // F
      printf("POS X %.2f Y %.2f Z %.2f\nANG Y %.2f X %.2f\n\n", camera.pos[0], camera.pos[1], camera.pos[2], camera.ang[0] / PI, camera.ang[1] / PI);
      break;
  }
}
