#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include <unistd.h>

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

#define WATER_CELL 0
#define SOLID_CELL 1

typedef struct {
  int type;
  /* 0 (empty) & 1 (full) */
  double fill_level;
  int x;
  int y;
} Cell;

typedef struct
{
  double flow_left;
  double flow_right;
  double flow_up;
  double flow_down;
} CellFlow;

void draw_cell(SDL_Surface *surface, Cell cell) {
  int pixel_x = cell.x * CELL_SIZE;
  int pixel_y = cell.y * CELL_SIZE;
  SDL_Rect cell_rect = (SDL_Rect){pixel_x, pixel_y, CELL_SIZE, CELL_SIZE};
  // * default background color
  SDL_FillRect(surface, &cell_rect, COLOR_BLACK);
  // * Water fill level
  if (cell.type == WATER_CELL) {
    int water_height = cell.fill_level > 1 ? CELL_SIZE : cell.fill_level * CELL_SIZE;
    // printf("%d \n", water_height);
    int empty_height = CELL_SIZE - water_height;
    SDL_Rect water_rect = (SDL_Rect){
        pixel_x,
        pixel_y + empty_height,
        CELL_SIZE,
        water_height};
    SDL_FillRect(surface, &water_rect, COLOR_BLUE);
  }
  // * Solid blocks
  if(cell.type == SOLID_CELL) {
    SDL_FillRect(surface, &cell_rect, COLOR_WHITE);
  }
}

void draw_grid(SDL_Surface *surface) {
  for (int i = 0; i < COLUMNS; i++) {
    SDL_Rect column = (SDL_Rect){
        i * CELL_SIZE,
        0,
        LINE_WIDTH,
        SCREEN_HEIGHT};
    SDL_FillRect(surface, &column, COLOR_GRAY);
  }
  for (int i = 0; i < ROWS; i++) {
    SDL_Rect row = (SDL_Rect){
        0,
        i * CELL_SIZE,
        SCREEN_WIDTH,
        LINE_HEIGHT};
    SDL_FillRect(surface, &row, COLOR_GRAY);
  }
}

// * Initialize the grid environment
void initialize_environment(Cell environment[ROWS * COLUMNS]) {
  for (int i = 0; i < ROWS; i++) {
    for(int j = 0; j < COLUMNS; j++) {
      // * Representing 2d grid in 1d array
      environment[j + COLUMNS * i] = (Cell){WATER_CELL, 0, j, i};
    }
  }
}

// * Update the environment
void draw_environment(SDL_Surface *surface, Cell environment[ROWS * COLUMNS]) {
  for (int i = 0; i < ROWS * COLUMNS; i++) {
    draw_cell(surface, environment[i]);
  }
}

// * Water flows down
void simulation_phase_rule1(Cell environment[ROWS * COLUMNS]) {
  // * Initialize next enviromnent state
  Cell environment_next[ROWS * COLUMNS];
  for (int i = 0; i < ROWS * COLUMNS; i++) {
    environment_next[i] = environment[i];
  }

  for (int i = 0; i < ROWS; ++i) {
    for (int j = 0; j < COLUMNS; ++j) {
      Cell src_cell = environment[j + COLUMNS * i];
      if (src_cell.type == WATER_CELL && i < ROWS - 1) {

        Cell dst_cell = environment[j + COLUMNS * (i + 1)];
        // * if below cell has some space & above cell has water then 
        // * let the water fall to below cell
        // if(dst_cell.fill_level < src_cell.fill_level) {
        if(dst_cell.fill_level < 1.0 && src_cell.fill_level > 0.02) {
          double water_needed = 1.0 - environment[j + COLUMNS * (i + 1)].fill_level;
          if (water_needed >= src_cell.fill_level && src_cell.fill_level > 0.0) {
            printf("Add water: %f %f\n", water_needed, src_cell.fill_level);
            environment_next[j + COLUMNS * i].fill_level = 0;
            environment_next[j + COLUMNS * (i + 1)].fill_level += environment[j + COLUMNS * i].fill_level;
          }
          else {
            // * If src_cell has fluid
            environment_next[j + COLUMNS * i].fill_level -= water_needed;
            // environment_next[j + COLUMNS * (i + 1)].fill_level += 1;
            environment_next[j + COLUMNS * (i + 1)].fill_level += water_needed;
          }
        }
      }
    }
  }

  for (int i = 0; i < ROWS * COLUMNS; ++i) {
    environment[i] = environment_next[i];
  }
}

// * Rule 2. Water flowing left & right
void simulation_phase_rule2(Cell environment[ROWS * COLUMNS]) {
  // * Initialize next enviromnent state
  Cell environment_next[ROWS * COLUMNS];
  for (int i = 0; i < ROWS * COLUMNS; i++) {
    environment_next[i] = environment[i];
  }

 for (int i = 0; i < ROWS; ++i) {
    for (int j = 0; j < COLUMNS; ++j) {
      Cell src_cell = environment[j + COLUMNS * i];
      // * If we hit the floor or below cell has fill_level >= 1 or below cell is solid cell
      if (i + 1 == ROWS ||
          environment[j + COLUMNS * (i + 1)].fill_level >= 1.0 ||
          environment[j + COLUMNS * (i + 1)].type == SOLID_CELL)
      {
        // * flow fluid to left
        if (src_cell.type == WATER_CELL && j > 0) {
          Cell dst_cell_left = environment[(j - 1) + COLUMNS * i];
          if (dst_cell_left.type == WATER_CELL && dst_cell_left.fill_level < src_cell.fill_level) {
            double delta_fill = (src_cell.fill_level - dst_cell_left.fill_level) / 3;
            // printf("%f %f\n", src_cell.fill_level, dst_cell_left.fill_level);
            environment_next[j + COLUMNS * i].fill_level -= delta_fill;
            environment_next[(j - 1) + COLUMNS * i].fill_level += delta_fill;
            // printf("Add left: %f\n", delta_fill);
          }
        }

        // * flow fluid to right
        if (src_cell.type == WATER_CELL && j < COLUMNS - 1) {
          Cell dst_cell_right = environment[(j + 1) + COLUMNS * i];
            if (dst_cell_right.type == WATER_CELL && dst_cell_right.fill_level < src_cell.fill_level) {
              double delta_fill = (src_cell.fill_level - dst_cell_right.fill_level) / 3;
              environment_next[j + COLUMNS * i].fill_level -= delta_fill;
              environment_next[(j + 1) + COLUMNS * i].fill_level += delta_fill;
              // printf("Add right: %f\n", delta_fill);
            }
          // printf("%f %f\n", src_cell.fill_level, delta_fill);
        }
      }
    }
  }
 
  for (int i = 0; i < ROWS * COLUMNS; ++i) {
    environment[i] = environment_next[i];
  }
}

void simulation_step(Cell environment[ROWS * COLUMNS]) {
  // * Rule 1. Water flows down
  simulation_phase_rule1(environment);

  // * Rule 2. Water flowing left & right
  simulation_phase_rule2(environment);
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

  // * Model the cell grid
  Cell environment[ROWS * COLUMNS];
  initialize_environment(environment);

  // * Poll for events
  bool quit = false;
  bool delete_mode = false;
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
            double fill_level = delete_mode ? 0 : 1;
            if(delete_mode != 0) {
              current_cell_type = WATER_CELL;
            }
            // printf("%d %d\n", cell_x, cell_y);
            Cell cell = {current_cell_type, fill_level, cell_x, cell_y};
            environment[cell_x + COLUMNS * cell_y] = cell;
            // draw_cell(surface, cell);
          }
  
        } break;
        case SDL_MOUSEBUTTONDOWN: {
          int cell_x = event.motion.x / CELL_SIZE;
          int cell_y = event.motion.y / CELL_SIZE; 
          // printf("%d %d\n", cell_x, cell_y);
          Cell cell = {WATER_CELL, 1, cell_x, cell_y};
          environment[cell_x + COLUMNS * cell_y] = cell;
        }
        break;
        case SDL_KEYDOWN: {
          switch(event.key.keysym.sym) {
            case SDLK_SPACE: {
              current_cell_type = !current_cell_type;
            } break;
            case SDLK_BACKSPACE: {
              delete_mode = !delete_mode;
            } break;
          }
        } break;
      }
 
      // * Perform simulation steps
      simulation_step(environment);     

      // * Draw updates
      draw_environment(surface, environment);
      draw_grid(surface);  
      SDL_UpdateWindowSurface(window);
      // SDL_Delay(10);
    }
  }
  SDL_Quit();
}
