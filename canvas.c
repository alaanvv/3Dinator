#include <SDL2/SDL.h>
#include <math.h>

typedef struct {
  SDL_Window* window;
  SDL_Renderer* renderer;

  unsigned int width;
  unsigned int height;
  double scale_x;
  double scale_y;

  char title[30];
} Canvas;

typedef struct {
  unsigned int R;
  unsigned int G;
  unsigned int B;
  unsigned int A;
} Color;

typedef double Point[2];

// ---

void canvas_init(Canvas *canvas);
void canvas_clear(Canvas canvas);
void canvas_display(Canvas canvas);
void canvas_delay(int ms);
void canvas_color(Canvas canvas, Color color);
void canvas_dot(Canvas canvas, Point point);
void canvas_line(Canvas canvas, Point from, Point to);
void canvas_triangle(Canvas canvas, Point v1, Point v2, Point v3);
void canvas_fill_triangle(Canvas canvas, Point v1, Point v2, Point v3);
void canvas_fill_flat_top_triangle(Canvas canvas, Point v1, Point v2, Point v3);
void canvas_fill_flat_bottom_triangle(Canvas canvas, Point v1, Point v2, Point v3);

// ---

void canvas_init(Canvas *canvas) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(canvas->width * canvas->scale_x, canvas->height * canvas->scale_y, 0, &(canvas->window), &(canvas->renderer));
  SDL_RenderSetScale(canvas->renderer, canvas->scale_x, canvas->scale_y);
  SDL_SetWindowTitle(canvas->window, canvas->title);

  canvas_color(*canvas, (Color) { 255, 255, 255, 255 });
}

void canvas_clear(Canvas canvas) {
  SDL_RenderClear(canvas.renderer);
}

void canvas_display(Canvas canvas) {
  SDL_RenderPresent(canvas.renderer);
}

void canvas_delay(int ms) {
  SDL_Delay(ms);
}

void canvas_color(Canvas canvas, Color color) {
  SDL_SetRenderDrawColor(canvas.renderer, color.R, color.G, color.B, color.A);
}

void canvas_dot(Canvas canvas, Point point) {
  if (0 <= point[0] < canvas.width && 0 <= point[1] < canvas.height)
    SDL_RenderDrawPoint(canvas.renderer, point[0], point[1]);
}

void canvas_line(Canvas canvas, Point from, Point to) {
  int dx = to[0] - from[0];
  int dy = to[1] - from[1];
  int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

  double x_inc = (double) dx / steps;
  double y_inc = (double) dy / steps;

  // TODO Shift the line to inbound if it's not
  Point point = { from[0], from[1] };
  for (int i = 0; i <= steps; i++) {
    canvas_dot(canvas, point);

    if (i != steps) {
      point[0] += x_inc;
      point[1] += y_inc;
    }

    if (point[0] < 0 || point[0] > canvas.width || point[1] < 0 || point[1] > canvas.height) break;
  }
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

  if (v2[1] < v1[1]) swap(v1, v2);
  if (v3[1] < v2[1]) swap(v2, v3);
  if (v2[1] < v1[1]) swap(v1, v2);

  if ((int) v1[1] == (int) v2[1]) canvas_fill_flat_top_triangle(canvas, v1, v2, v3);
  else if ((int) v2[1] == (int) v3[1]) canvas_fill_flat_bottom_triangle(canvas, v1, v2, v3);
  else {
    Point v4;
    v4[1] = v2[1];
    v4[0] = v1[0] + ((v3[0] - v1[0]) * ((v4[1] - v1[1]) / (v3[1] - v1[1])));

    canvas_fill_flat_top_triangle(canvas, v2, v4, v3);
    canvas_fill_flat_bottom_triangle(canvas, v1, v2, v4);
  }
}

void canvas_fill_flat_top_triangle(Canvas canvas, Point v1, Point v2, Point v3) {
  if (v2[0] < v1[0]) {
    Point temp;
    temp[0] = v1[0]; temp[1] = v1[1];
    v1[0] = v2[0]; v1[1] = v2[1];
    v2[0] = temp[0]; v2[1] = temp[1];
  }

  float x1_step = (v3[0] - v1[0]) / (v3[1] - v1[1]);
  float x2_step = (v3[0] - v2[0]) / (v3[1] - v1[1]);
  float x1_curr = v1[0];
  float x2_curr = v2[0];
  
  for (float y = v1[1]; y <= v3[1]; y++) {
    canvas_line(canvas, (Point) {x1_curr, y}, (Point) {x2_curr, y});
    x1_curr += x1_step;
    x2_curr += x2_step;
  }
}

void canvas_fill_flat_bottom_triangle(Canvas canvas, Point v1, Point v2, Point v3) {
  if (v3[0] < v2[0]) {
    Point temp;
    temp[0] = v2[0]; temp[1] = v2[1];
    v2[0] = v3[0]; v2[1] = v3[1];
    v3[0] = temp[0]; v3[1] = temp[1];
  }

  float x1_step = (v1[0] - v2[0]) / (v2[1] - v1[1]);
  float x2_step = (v1[0] - v3[0]) / (v2[1] - v1[1]);
  float x1_curr = v2[0];
  float x2_curr = v3[0];

  for (int y = v2[1]; y >= v1[1]; y--) {
    canvas_line(canvas, (Point) {x1_curr, y}, (Point) {x2_curr, y});
    x1_curr += x1_step;
    x2_curr += x2_step;
  }
}
