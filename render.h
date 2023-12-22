#include <math.h>
#include "canvas.h"

#define POW2(x) (x * x)

typedef float    f32;
typedef double   f64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef int8_t   i8;
typedef int16_t  i16;

typedef f64 Vec4[4];
typedef f64 Vec3[3];
typedef f64 Vec2[3];
typedef Vec3 Tri[3];
typedef Tri Mesh[];

typedef struct {
  f32 near;
  f32 far;
  f32 fov;
  f32 ratio;
  Vec2 proj_vec;
} View;

typedef struct {
  Vec3 pos;
  Vec2 ang;
} Camera;

// ---

void render_proj_vec(View* view) {
  f32 fov_rad = 1 / tan(view->fov * 0.5);
  f32 range = view->far - view->near;

  view->proj_vec[0] = view->ratio * fov_rad;
  view->proj_vec[1] = fov_rad;
}

void render_project(View view, Tri from, Tri to, u16 width, u16 height) {
  for (u8 i = 0; i < 3; i++) {
    to[i][0] =  from[i][0] * view.proj_vec[0];
    to[i][1] = -from[i][1] * view.proj_vec[1];

    if (from[i][2]) {
      to[i][0] /= from[i][2];
      to[i][1] /= from[i][2];
    }
    
    to[i][0] = trunc((to[i][0] + 1) * 0.5 * width);
    to[i][1] = trunc((to[i][1] + 1) * 0.5 * height);
  }
}

void render_create_normal(Tri tri, Vec3 normal) {
  Vec3 l1 = {
    tri[1][0] - tri[0][0],
    tri[1][1] - tri[0][1],
    tri[1][2] - tri[0][2]
  };

  Vec3 l2 = {
    tri[2][0] - tri[0][0],
    tri[2][1] - tri[0][1],
    tri[2][2] - tri[0][2]
  };

  normal[0] = l1[1] * l2[2] - l1[2] * l2[1];
  normal[1] = l1[2] * l2[0] - l1[0] * l2[2];
  normal[2] = l1[0] * l2[1] - l1[1] * l2[0];

  f32 l = sqrt(POW2(normal[0]) + POW2(normal[1]) + POW2(normal[2]));
  normal[0] /= l;
  normal[1] /= l;
  normal[2] /= l;
}

void render_translate(Tri tri, Vec3 vec) {
  tri[0][0] += vec[0];
  tri[1][0] += vec[0];
  tri[2][0] += vec[0];

  tri[0][1] += vec[1];
  tri[1][1] += vec[1];
  tri[2][1] += vec[1];

  tri[0][2] += vec[2];
  tri[1][2] += vec[2];
  tri[2][2] += vec[2];
}

void render_rotate(Tri tri, Vec3 vec) {
  for (u8 i = 0; i < 3; i++) {
    Vec3 buffer = { tri[i][0], tri[i][1], tri[i][2] };

      f32 cos_t = cos(vec[0]), sin_t = sin(vec[0]);
      tri[i][1] =  cos_t * buffer[1] + sin_t * buffer[2];
      tri[i][2] = -sin_t * buffer[1] + cos_t * buffer[2];

      buffer[1] = tri[i][1];
      buffer[2] = tri[i][2];
    if (vec[1]) {
      f32 cos_t = cos(vec[1]), sin_t = sin(vec[1]);
      tri[i][0] = cos_t * tri[i][0] + -sin_t * tri[i][2];
      tri[i][2] = sin_t * buffer[0] +  cos_t * tri[i][2];

      buffer[0] = tri[i][0];
      buffer[2] = tri[i][2];
    }
    if (vec[2]) {
      f32 cos_t = cos(vec[2]), sin_t = sin(vec[2]);
      tri[i][0] =  cos_t * tri[i][0] + sin_t * tri[i][1];
      tri[i][1] = -sin_t * buffer[0] + cos_t * tri[i][1];
    }
  }
}

void render_scale(Tri tri, Vec3 vec) {
  tri[0][0] *= vec[0];
  tri[1][0] *= vec[0];
  tri[2][0] *= vec[0];

  tri[0][1] *= vec[1];
  tri[1][1] *= vec[1];
  tri[2][1] *= vec[1];

  tri[0][2] *= vec[2];
  tri[1][2] *= vec[2];
  tri[2][2] *= vec[2];
}
