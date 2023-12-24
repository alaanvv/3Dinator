#include <SDL2/SDL.h>
#include <math.h>

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define AMAX(x, y) (abs(x) > abs(y) ? abs(x) : abs(y))

typedef char     c8;
typedef float    f32;
typedef double   f64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef int8_t   i8;
typedef int16_t  i16;

typedef struct {
  SDL_Window* window;
  SDL_Renderer* renderer;

  u16 width;
  u16 height;
  f32 scale_x;
  f32 scale_y;
} Canvas;

typedef struct { u8 R; u8 G; u8 B; u8 A; } Color;
typedef i16 iVec2[3];
typedef f64 Point[2];

static inline void canvas_color(Canvas canvas, Color color) { SDL_SetRenderDrawColor(canvas.renderer, color.R, color.G, color.B, color.A); }
static inline void canvas_dot(Canvas canvas, Point point)   { SDL_RenderDrawPoint(canvas.renderer, point[0], point[1]); }
static inline void canvas_title(Canvas canvas, c8 title[])  { SDL_SetWindowTitle(canvas.window, title); }
static inline void canvas_display(Canvas canvas)            { SDL_RenderPresent(canvas.renderer); }
static inline void canvas_clear(Canvas canvas)              { SDL_RenderClear(canvas.renderer); }
static inline void canvas_delay(u16 ms)                     { SDL_Delay(ms); }

void canvas_init(Canvas* canvas) {
  SDL_CreateWindowAndRenderer(canvas->width * canvas->scale_x, canvas->height * canvas->scale_y, 0, &(canvas->window), &(canvas->renderer));
  SDL_RenderSetScale(canvas->renderer, canvas->scale_x, canvas->scale_y);
  SDL_Init(SDL_INIT_VIDEO);
}

void canvas_update(Canvas canvas, u16 delay) {
  canvas_display(canvas);
  canvas_color(canvas, (Color) {0, 0, 0, 255});
  canvas_clear(canvas);
  canvas_delay(delay);
}

void canvas_line(Canvas canvas, Point from, Point to) {
  i16 dx = to[0] - from[0];
  i16 dy = to[1] - from[1];
  i16 steps = AMAX(dx, dy);

  f32 x_inc = (f32) dx / steps;
  f32 y_inc = (f32) dy / steps;

  Point point = { from[0], from[1] };
  for (i16 i = 0; i <= steps; i++) {
    canvas_dot(canvas, point);

    point[0] += x_inc;
    point[1] += y_inc;
  }
}

void canvas_straight_line(Canvas canvas, Point from, i16 x2) {
  i16 x = from[0];
  while (x <= x2) canvas_dot(canvas, (Point) { x++, from[1] });
}

void canvas_triangle(Canvas canvas, Point v1, Point v2, Point v3) {
  canvas_line(canvas, v1, v2);
  canvas_line(canvas, v2, v3);
  canvas_line(canvas, v3, v1);
}

void canvas_fill_triangle(Canvas canvas, Point v1, Point v2, Point v3) {
  void swap(Point v1, Point v2) {
    Point temp;
    temp[0] = v1[0]; temp[1] = v1[1];
    v1[0] = v2[0]; v1[1] = v2[1];
    v2[0] = temp[0]; v2[1] = temp[1];
  }

  void fill(Point v1, Point v2, Point v3, u8 top) {
    if (v3[1] < 0 || v1[1] > canvas.height) return;

    Point lef, mid, rig;
    if (top) {
      lef[0] = v1[0]; lef[1] = v1[1];
      mid[0] = v3[0]; mid[1] = v3[1];
      rig[0] = v2[0]; rig[1] = v2[1];
    }
    else {
      lef[0] = v2[0]; lef[1] = v2[1];
      mid[0] = v1[0]; mid[1] = v1[1];
      rig[0] = v3[0]; rig[1] = v3[1];
    }
    if (rig[0] < lef[0]) swap(lef, rig);

    f32 x1_step = (mid[0] - lef[0]) / (v3[1] - v1[1]);
    f32 x2_step = (mid[0] - rig[0]) / (v3[1] - v1[1]);
    f32 x1 = lef[0];
    f32 x2 = rig[0];

    if (top ? lef[1] < 0 : lef[1] > canvas.height) {
      u16 dis = top ? 0 - lef[1] : lef[1] - canvas.height;
      lef[1] = top ? 0 : canvas.height;
      x1 += x1_step * dis;
      x2 += x2_step * dis;
    }

    mid[1] = top ? MIN(mid[1], canvas.height) : MAX(mid[1], 0);
    for (i16 y = lef[1]; top ? (y <= mid[1]) : (y >= mid[1]); top ? y++ : y--) {
      if (x2 > 0 && x1 < canvas.width)
        canvas_straight_line(canvas, (Point) {MAX(x1, 0), y}, MIN(x2, canvas.width));
      x1 += x1_step;
      x2 += x2_step;
    }
  }

  if (v2[1] < v1[1]) swap(v1, v2);
  if (v3[1] < v2[1]) {
    swap(v2, v3);
    if (v2[1] < v1[1]) swap(v1, v2);
  }

  if (v1[1] == v2[1]) fill(v1, v2, v3, 1);
  else if (v2[1] == v3[1]) fill(v1, v2, v3, 0);
  else {
    Point v4;
    v4[1] = v2[1];
    v4[0] = v1[0] + ((v3[0] - v1[0]) * ((v4[1] - v1[1]) / (v3[1] - v1[1])));

    fill(v2, v4, v3, 1);
    fill(v1, v2, v4, 0);
  }
}
