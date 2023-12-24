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
#define INTERVAL 3e1

typedef float    f32;
typedef double   f64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   i8;
typedef int16_t  i16;

u8 is_inside_triangle(i16 x, i16 y, Tri tri);
f32 cross_product_2d(Vec3 v1, Vec3 v2);
f32 dot_product(Vec3 v1, Vec3 v2);

// ---

Canvas canvas = { NULL, NULL, WIDTH, HEIGHT, SCALE_X, SCALE_Y };
View view =     { 0.01, 1000, PI2, WIDTH / HEIGHT };
Camera cam =    { 0, 0, -9 };

f32 ry;
Tri tri, tri_proj;
Vec3 normal;

#include "control.h"

// ---

void loop() {
  ry += TAU / 1e3;

  for (u16 i = 0; i < LEN(OBJ); i++) {
    for (u8 p = 0; p < 3; p++) 
      for (u8 c = 0; c < 3; c++)
        tri[p][c] = OBJ[i][p][c];

    // Place object and rotate
    render_rotate(tri, (Vec3) { PI4, ry, 0 });

    // Move world based on cam position
    render_translate(tri, (Vec3) { -cam.pos[0], -cam.pos[1], -cam.pos[2] });
    render_rotate(tri, (Vec3) { 0, 0, -cam.ang[1] * sin(cam.ang[0]) });
    render_rotate(tri, (Vec3) { -cam.ang[1] * cos(cam.ang[0]), -cam.ang[0], 0 });

    render_create_normal(tri, normal);
    if (normal[0] * tri[0][0] + normal[1] * tri[0][1] + normal[2] * tri[0][2] > 0 || tri[0][2] < view.near && tri[1][2] < view.near && tri[2][2] < view.near) continue;

    u8 light = 255 * (dot_product(normal, (Vec3) { 0, 0, -0.8 }) + 1) * 0.5;
    canvas_color(canvas, (Color) { light, light, light, 255 });
    render_project(view, tri, tri_proj, canvas.width, canvas.height);
    canvas_fill_triangle(canvas, tri_proj[0], tri_proj[1], tri_proj[2]); 

    int mouse_x, mouse_y;
    u8 mouse_state = SDL_BUTTON(SDL_GetMouseState(&mouse_x, &mouse_y));
    if (mouse_state == 1) printf("aha\n");
    mouse_x /= SCALE_X;
    mouse_y /= SCALE_Y;

    if (is_inside_triangle(mouse_x, mouse_y, tri_proj)) {
      canvas_color(canvas, (Color) { light * 0.5, light * 0.5, light * 0.5, 255});
      canvas_fill_triangle(canvas, tri_proj[0], tri_proj[1], tri_proj[2]); 
    }
  }
}

u8 main() {
  render_proj_vec(&view);
  canvas_init(&canvas);

  SDL_Event ev;
  while (1) {
    while (SDL_PollEvent(&ev))
      if (ev.type == SDL_QUIT) return 0; 
      else if (ev.type == SDL_KEYDOWN) handle_key(ev);

    canvas_update(canvas, INTERVAL);
    loop();
  }
}

inline f32 dot_product(Vec3 v1, Vec3 v2) { return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]; }
inline f32 cross_product_2d(Vec3 v1, Vec3 v2) { return v1[0] * v2[1] - v1[1] * v2[0]; }

u8 is_inside_triangle(i16 x, i16 y, Tri tri) {
  // c stands for clockwise, 
  // cuz apparently somehow 
  // I did the mesh wrong and 
  // it passed by the first 
  // normal check
  Vec3 n1c = { tri[1][0] - tri[0][0], tri[1][1] - tri[0][1] };
  Vec3 n2c = { tri[2][0] - tri[1][0], tri[2][1] - tri[1][1] };
  Vec3 n3c = { tri[0][0] - tri[2][0], tri[0][1] - tri[2][1] };
  Vec3 n1 = { tri[0][0] - tri[1][0], tri[0][1] - tri[1][1] };
  Vec3 n2 = { tri[1][0] - tri[2][0], tri[1][1] - tri[2][1] };
  Vec3 n3 = { tri[2][0] - tri[0][0], tri[2][1] - tri[0][1] };

  Vec3 pn1 = { x - tri[0][0], y - tri[0][1] };
  Vec3 pn2 = { x - tri[1][0], y - tri[1][1] };
  Vec3 pn3 = { x - tri[2][0], y - tri[2][1] };

  return (cross_product_2d(n1c, pn1) >= 0 && cross_product_2d(n2c, pn2) >= 0 && cross_product_2d(n3c, pn3) >= 0 ||
          cross_product_2d(n1, pn1) >= 0 && cross_product_2d(n2, pn2) >= 0 && cross_product_2d(n3, pn3) >= 0);
}
