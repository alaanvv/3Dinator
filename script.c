#include <time.h>
#include "render.h"
#include "mesh.h"

#define LEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define PI 3.1415962
#define TAU PI * 2
#define PI2 PI * 0.5
#define PI4 PI * 0.25

#define OBJ bottle 
#define WIDTH 100
#define HEIGHT 100
#define SCALE_X 10
#define SCALE_Y 10

typedef float    f32;
typedef double   f64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   i8;
typedef int16_t  i16;

Canvas canvas = { NULL, NULL, WIDTH, HEIGHT, SCALE_X, SCALE_Y };
Camera camera = { { 0, 0, 0 }, 0 };
View view =     { 0.1, 1000, PI2, WIDTH / HEIGHT };

// ---

u8 main() {
  render_proj_vec(&view);
  canvas_init(&canvas);

  Tri tri, tri_proj;
  Vec3 normal;
  f32 ry;

  SDL_Event e;
  while (1) {
    while(SDL_PollEvent(&e)) { if (e.type == SDL_QUIT) return 0; }
    canvas_update(canvas, 3e0);

    ry += TAU / 1e3;

    for (u16 i = 0; i < LEN(OBJ); i++) {
      for (u8 p = 0; p < 3; p++) 
        for (u8 c = 0; c < 3; c++)
          tri[p][c] = OBJ[i][p][c];

      scale(tri, (Vec3) { .3, .3, .3 });
      rotate(tri, (Vec3) { PI4, ry, 0 });
      translate(tri, (Vec3) { 0, 0, 3 });

      render_create_normal(tri, normal);
      if (normal[0] * (tri[0][0] - camera.pos[0]) + normal[1] * (tri[0][1] - camera.pos[1]) + normal[2] * (tri[0][2] - camera.pos[2]) > 0) continue;
      u8 light = 255 / 2 * ((normal[0] + 1));

      render_project(view, tri, tri_proj, canvas.width, canvas.height);
      canvas_color(canvas, (Color) { light, light, light, 255 });
      canvas_fill_triangle(canvas, tri_proj[0], tri_proj[1], tri_proj[2]); 
    }
  }

  return 0;
}
