#include <math.h>
#include <time.h>
#include "canvas.h"

#define len(arr) (sizeof(arr) / sizeof(arr[0]))
#define PI 3.1415962
#define PI2 PI * 2

typedef double Mat4[4][4];
typedef double Vec3[3];
typedef Vec3 Tri[3];
typedef Tri Mesh[];

typedef struct {
  float near;
  float far;
  float fov;
  float ratio;
} View;

typedef struct {
  Vec3 pos;
  float angle;
} Camera;

void create_projection_matrix(View view, Mat4 to);
void scale_to_view(Tri tri, int width, int height);
void create_projection(Mat4 proj, Tri from, Tri to);
void create_normal(Tri tri, Vec3 normal);
void translate(Tri tri, Vec3 vec);
void rotate(Tri tri, Vec3 vec);

// ---

#include "mesh.c"

Color white = { 255, 255, 255, 255 };
Color black = { 0, 0, 0, 255 };

Canvas canvas = { NULL, NULL, 250, 250, 4, 4, "" };
View view = { 0.1, 1000, 90 };
Camera camera = { { 0, 0, 0 }, 0 };
Mat4 matproj;

// ---

int main() {
  view.ratio = canvas.width / canvas.height;
  create_projection_matrix(view, matproj);
  canvas_init(&canvas);

  double xr = 0, yr = 0, zr = 0;
  SDL_Event e;
  int r = 1;
  while (r) {
    while(SDL_PollEvent(&e)) { if (e.type == SDL_QUIT) { r = 0; } }

    for (int i = 0; i < len(cube); i++) {
      Tri tri, tri_proj;
      for (int p = 0; p < 3; p++) 
        for (int c = 0; c < 3; c++)
          tri[p][c] = cube[i][p][c];

      Vec3 rotate_vec = { xr, yr, zr };
      Vec3 translate_vec = { -0.5, -0.5, 3 };
      xr += PI2 / 1000;
      yr += PI2 / 10000;
      zr += PI2 / 100000;


      rotate(tri, rotate_vec);
      translate(tri, translate_vec);

      Vec3 n;
      create_normal(tri, n);
      if (n[0] * (tri[0][0] - camera.pos[0]) + n[1] * (tri[0][1] - camera.pos[1]) + n[2] * (tri[0][2] - camera.pos[2]) > 0) continue;

      create_projection(matproj, tri, tri_proj);

      scale_to_view(tri_proj, canvas.width, canvas.height);
      canvas_color(canvas, white);
      canvas_fill_triangle(canvas, tri_proj[0], tri_proj[1], tri_proj[2]); 
    }

    canvas_display(canvas);
    canvas_delay(50);
    canvas_color(canvas, black);
    canvas_clear(canvas);
  }

  return 0;
}

// ---

void create_projection_matrix(View view, Mat4 to) {
  float fov_rad = 1 / tan(view.fov * 0.5 / 180 * 3.14159);
  float range = view.far - view.near;

  to[0][0] = view.ratio * fov_rad;
  to[1][1] = fov_rad;
  to[2][2] = view.far / range;
  to[2][3] = 1;
  to[3][2] = -view.far * view.near / range;
}

void scale_to_view(Tri tri, int width, int height) {
  tri[0][0] = (tri[0][0] + 1) * 0.5 * width; 
  tri[0][1] = (tri[0][1] + 1) * 0.5 * height;
  tri[1][0] = (tri[1][0] + 1) * 0.5 * width; 
  tri[1][1] = (tri[1][1] + 1) * 0.5 * height;
  tri[2][0] = (tri[2][0] + 1) * 0.5 * width; 
  tri[2][1] = (tri[2][1] + 1) * 0.5 * height;
}

void create_projection(Mat4 proj, Tri from, Tri to) {
  for (int i = 0; i < 3; i++) {
    to[i][0] = from[i][0] * proj[0][0];
    to[i][1] = from[i][1] * proj[1][1];
    to[i][2] = from[i][2] * proj[2][2] + proj[3][2];

    if (!from[i][2]) return;
    to[i][0] /= from[i][2];
    to[i][1] /= from[i][2];
    to[i][2] /= from[i][2];
  }
}

void create_normal(Tri tri, Vec3 normal) {
      Vec3 l1, l2;

      l1[0] = tri[1][0] - tri[0][0];
      l1[1] = tri[1][1] - tri[0][1];
      l1[2] = tri[1][2] - tri[0][2];

      l2[0] = tri[2][0] - tri[0][0];
      l2[1] = tri[2][1] - tri[0][1];
      l2[2] = tri[2][2] - tri[0][2];

      normal[0] = l1[1] * l2[2] - l1[2] * l2[1];
      normal[1] = l1[2] * l2[0] - l1[0] * l2[2];
      normal[2] = l1[0] * l2[1] - l1[1] * l2[0];

      float l = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
      normal[0] /= l;
      normal[1] /= l;
      normal[2] /= l;
}

void translate(Tri tri, Vec3 vec) {
  if (vec[0]) {
    tri[0][0] += vec[0];
    tri[1][0] += vec[0];
    tri[2][0] += vec[0];
  }
  if (vec[1]) {
    tri[0][1] += vec[1];
    tri[1][1] += vec[1];
    tri[2][1] += vec[1];
  }
  if (vec[2]) {
    tri[0][2] += vec[2];
    tri[1][2] += vec[2];
    tri[2][2] += vec[2];
  }
}

void rotate(Tri tri, Vec3 vec) {
  for (int i = 0; i < 3; i++) {
    Vec3 buffer;
    buffer[0] = tri[i][0];
    buffer[1] = tri[i][1];
    buffer[2] = tri[i][2];

    if (vec[0]) {
      float cost = cos(vec[0]), sint = sin(vec[0]);
      tri[i][1] =  cost * buffer[1] + sint * buffer[2];
      tri[i][2] = -sint * buffer[1] + cost * buffer[2];

      buffer[1] = tri[i][1];
      buffer[2] = tri[i][2];
    }
    if (vec[1]) {
      float cost = cos(vec[1]), sint = sin(vec[1]);
      tri[i][0] = cost * tri[i][0] + -sint * tri[i][2];
      tri[i][2] = sint * buffer[0] +  cost * tri[i][2];

      buffer[0] = tri[i][0];
      buffer[2] = tri[i][2];
    }
    if (vec[2]) {
      float cost = cos(vec[2]), sint = sin(vec[2]);
      tri[i][0] =  cost * tri[i][0] + sint * tri[i][1];
      tri[i][1] = -sint * buffer[0] + cost * tri[i][1];

      buffer[0] = tri[i][0];
      buffer[1] = tri[i][1];
    }
  }
}

