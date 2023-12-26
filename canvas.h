#include <SDL2/SDL.h>
#include <math.h>

typedef struct {
  SDL_Window* window;
  SDL_Renderer* renderer;

  uint16_t width;
  uint16_t height;
  float scale_x;
  float scale_y;
} Canvas;

typedef int16_t Tri2D[3][2];
typedef struct { uint8_t R; uint8_t G; uint8_t B; uint8_t A; } Color;

static inline void canvas_title(Canvas canvas, char title[]) { SDL_SetWindowTitle(canvas.window, title); }
static inline void canvas_color(Canvas canvas, Color color)  { SDL_SetRenderDrawColor(canvas.renderer, color.R, color.G, color.B, color.A); }
static inline void canvas_display(Canvas canvas)             { SDL_RenderPresent(canvas.renderer); }
static inline void canvas_clear(Canvas canvas)               { SDL_RenderClear(canvas.renderer); }
static inline void canvas_delay(uint16_t ms)                 { SDL_Delay(ms); }

static inline void canvas_init(Canvas* canvas) {
  SDL_CreateWindowAndRenderer(canvas->width * canvas->scale_x, canvas->height * canvas->scale_y, 0, &(canvas->window), &(canvas->renderer));
  SDL_RenderSetScale(canvas->renderer, canvas->scale_x, canvas->scale_y);
  SDL_Init(SDL_INIT_VIDEO);
}

static inline void canvas_update(Canvas canvas, uint16_t delay) {
  canvas_display(canvas);
  canvas_color(canvas, (Color) {0, 0, 0, 255});
  canvas_clear(canvas);
  canvas_delay(delay);
}

static inline void canvas_triangle(Canvas canvas, Tri2D tri, uint8_t light) {
    SDL_Vertex vertices[3] = {
      {{ tri[0][0], tri[0][1] }, { light, light, light, 255 }, { 1, 1 }},
      {{ tri[1][0], tri[1][1] }, { light, light, light, 255 }, { 1, 1 }},
      {{ tri[2][0], tri[2][1] }, { light, light, light, 255 }, { 1, 1 }}
    };

    SDL_RenderGeometry(canvas.renderer, NULL, vertices, 3, NULL, 0);
}
