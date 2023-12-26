#include <SDL2/SDL.h>
#include <math.h>

typedef struct {
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;

  uint16_t width;
  uint16_t height;
  float scale_x;
  float scale_y;
  uint16_t* z_buffer;
} Canvas;

typedef int16_t Tri2D[3][2]; // Still have a Z for depth-buffering
typedef struct { uint8_t R; uint8_t G; uint8_t B; uint8_t A; } Color;


static inline void canvas_title(Canvas canvas, char title[]) { SDL_SetWindowTitle(canvas.window, title); }
static inline void canvas_color(Canvas canvas, Color color)  { SDL_SetRenderDrawColor(canvas.renderer, color.R, color.G, color.B, color.A); }
static inline void canvas_display(Canvas canvas)             { SDL_RenderPresent(canvas.renderer); }
static inline void canvas_clear(Canvas canvas)               { SDL_RenderClear(canvas.renderer); }
static inline void canvas_delay(uint16_t ms)                 { SDL_Delay(ms); }

static inline void canvas_init(Canvas* canvas) {
  SDL_CreateWindowAndRenderer(canvas->width * canvas->scale_x, canvas->height * canvas->scale_y, SDL_WINDOW_ALWAYS_ON_TOP, &(canvas->window), &(canvas->renderer));
  canvas->texture = SDL_CreateTexture(canvas->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, canvas->width, canvas->height);
  SDL_Init(SDL_INIT_EVERYTHING);

  int16_t z_buffer[canvas->width * canvas->height];
  canvas->z_buffer = &z_buffer[0];
}

static inline void canvas_update(Canvas canvas, uint16_t delay) {
  SDL_SetRenderTarget(canvas.renderer, NULL);
  SDL_RenderCopy(canvas.renderer, canvas.texture, NULL, NULL);
  canvas_display(canvas);

  SDL_SetRenderTarget(canvas.renderer, canvas.texture);
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
