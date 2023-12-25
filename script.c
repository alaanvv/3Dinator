#include "render.h"
#include "mesh.h"

#define LEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define PI 3.1415962
#define TAU PI * 2
#define PI2 PI * 0.5
#define PI4 PI * 0.25

#define OBJ bottle 
#define WIDTH 1000
#define HEIGHT 1000
#define SCALE_X 1
#define SCALE_Y 1
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
typedef int32_t  i32;

void copy_triangle_to_buffer(Tri tri);
void transform_against_camera(Tri tri);
u8  behind_camera(Tri tri);
f32 dot_product(Vec3 v1, Vec3 v2);

// ---

Canvas canvas = { NULL, NULL, WIDTH, HEIGHT, SCALE_X, SCALE_Y };
View view =     { 0.01, 1000, PI2, WIDTH / HEIGHT };
Camera cam =    { 0, 0, 0 };

Tri tri, tri_proj;
Vec3 normal;
u8 light;

#include "control.h"

// ---

void loop() {
  for (u16 i = 0; i < LEN(cube); i++) {
    copy_triangle_to_buffer(cube[i]);
    transform_against_camera(tri);

    render_create_normal(tri, normal);
    if (behind_camera(tri)) continue;

    light = 255 * (dot_product(normal, (Vec3) { 0, 0, -0.8 }) + 1) * 0.5;
    canvas_color(canvas, (Color) { light, light, light, 255 });
    render_project(view, tri, tri_proj, canvas.width, canvas.height);
    canvas_fill_triangle(canvas, tri_proj[0], tri_proj[1], tri_proj[2]); 
    canvas_color(canvas, (Color) { 0, 0, 0, 255 });
    canvas_triangle(canvas, tri_proj[0], tri_proj[1], tri_proj[2]); 
  }
}

void transform_against_camera(Tri tri) {
  render_translate(tri, (Vec3) { -cam.pos[0], -cam.pos[1], -cam.pos[2] });
  render_rotate(tri, (Vec3) { 0, 0, -cam.ang[1] * sin(cam.ang[0]) });
  render_rotate(tri, (Vec3) { -cam.ang[1] * cos(cam.ang[0]), -cam.ang[0], 0 });
}

void copy_triangle_to_buffer(Tri from) {
    for (u8 p = 0; p < 3; p++) 
      for (u8 c = 0; c < 3; c++)
        tri[p][c] = from[p][c];
}

inline u8  behind_camera(Tri tri) { return tri[0][2] < view.near && tri[1][2] < view.near && tri[2][2] < view.near; }
inline f32 dot_product  (Vec3 v1, Vec3 v2) { return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]; }

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
