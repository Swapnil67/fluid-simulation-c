#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_BLUE 0x34c3eb
#define COLOR_GRAY 0x1f1f1f1f

#define CELL_SIZE 20
#define LINE_WIDTH 2
#define LINE_HEIGHT 2
#define COLUMNS SCREEN_WIDTH / CELL_SIZE
#define ROWS SCREEN_HEIGHT / CELL_SIZE

#define SOLID_CELL 1
#define WATER_CELL 0

typedef struct {
  int type;
  int fill_level;
  int x;
  int y;
} Cell;

void draw_cell(SDL_Surface *surface, Cell cell) {
  int pixel_x = cell.x * CELL_SIZE;
  int pixel_y = cell.y * CELL_SIZE;
  SDL_Rect cell_rect = (SDL_Rect){pixel_x, pixel_y, CELL_SIZE, CELL_SIZE};
  Uint32 color = COLOR_WHITE;
  if (cell.type == WATER_CELL) {
    color = COLOR_BLUE;
  }
  SDL_FillRect(surface, &cell_rect, color);
}

void draw_grid(SDL_Surface *surface) {
  for (int i = 0; i < COLUMNS; ++i) {
    SDL_Rect column = (SDL_Rect){
        i * CELL_SIZE,
        0,
        LINE_WIDTH,
        SCREEN_HEIGHT};
    SDL_FillRect(surface, &column, COLOR_GRAY);
  }
  for (int i = 0; i < ROWS; ++i) {
    SDL_Rect row = (SDL_Rect){
        0,
        i * CELL_SIZE,
        SCREEN_WIDTH,
        LINE_HEIGHT};
    SDL_FillRect(surface, &row, COLOR_GRAY);
  }
}

int main() {
  printf("Fluid Simulation\n");
  SDL_Init(SDL_INIT_VIDEO);

  // * Create a SDL window
  SDL_Window *window = SDL_CreateWindow(
      "Fluid Simulation",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      SCREEN_WIDTH,
      SCREEN_HEIGHT,
      0);

  SDL_Surface *surface = SDL_GetWindowSurface(window);
  draw_grid(surface);  

  // * Poll for events
  bool quit = false;
  int current_cell_type = SOLID_CELL;
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
        } break;
        case SDL_MOUSEMOTION: {
          // * Mouse clicked
          if(event.motion.state != 0) {
            int cell_x = event.motion.x / CELL_SIZE;
            int cell_y = event.motion.y / CELL_SIZE;
            // printf("%d %d\n", cell_x, cell_y);
            Cell cell = {current_cell_type, 0, cell_x, cell_y};
            draw_cell(surface, cell);
          }
        } break;
        case SDL_KEYDOWN: {
          switch(event.key.keysym.sym) {
            case SDLK_SPACE: {
              current_cell_type = !current_cell_type;
            } break;
          }
        } break;
      }
      SDL_UpdateWindowSurface(window);
    }
  }
  SDL_Quit();
  // SDL_Delay(3000);
}
