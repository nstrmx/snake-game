#include "snake.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#define WINDOW_X 10
#define WINDOW_Y 10
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define GRID_SIZE 16
#define GRID_DIM 400

#define LOG(...) if(DEBUG) printf(__VA_ARGS__)

struct game_state{
    int score;
};
typedef struct game_state GameState;

typedef struct {
    int x;
    int y;
} position;

// == SNAKE ==

enum {
    SNAKE_UP,
    SNAKE_DOWN,
    SNAKE_LEFT,
    SNAKE_RIGHT,
};

struct snake_seg {
    position pos;
    struct snake_seg *prev;
};
typedef struct snake_seg SnakeSeg;

struct snake_state {
    int dir;
    struct snake_seg *head;
    struct snake_seg *tail;
};
typedef struct snake_state SnakeState;


SnakeState *init_snake() {
    SnakeSeg *seg = malloc(sizeof(SnakeSeg));
    seg->pos.x = rand() % (GRID_SIZE / 2) + (GRID_SIZE / 4);
    seg->pos.y = rand() % (GRID_SIZE / 2) + (GRID_SIZE / 4);
    seg->prev = NULL;

    SnakeState *state = malloc(sizeof(SnakeState));
    state->dir = SNAKE_UP;
    state->head = seg;
    state->tail = seg;
    
    LOG(
        "initted snake: x = %d; y = %d; dir = %d; head = %p; tail = %p;\n",
        seg->pos.x, seg->pos.y, state->dir, state->head, state->tail
    );
    return state;
}

void drop_snake(SnakeState *state) {
    SnakeSeg *track = state->tail;
    SnakeSeg *temp;
    while(track != NULL) {
        temp = track;
        track = track->prev;
        free(temp);
    }
    LOG("dropped snake\n");
    return;
}

void increase_snake(SnakeState *state, position pos) {
    struct snake_seg *new = malloc(sizeof(struct snake_seg));
    
    new->pos = pos;
    
    state->head->prev = new;
    state->head = new;
    state->head->prev = NULL;
    
    LOG(
        "increased snake: x = %d; y = %d; dir = %d; head = %p; head.prev = %p; tail = %p; tail.prev = %p\n",
        new->pos.x, new->pos.y, state->dir, state->head, state->head->prev, state->tail, state->tail->prev
    );
    return;
}

position next_pos(SnakeState *state) {
    position pos;
    switch(state->dir) {
        case SNAKE_UP:
            pos.x = state->head->pos.x;
            pos.y = state->head->pos.y - 1;
            break;
        case SNAKE_DOWN:
            pos.x = state->head->pos.x;
            pos.y = state->head->pos.y + 1;
            break;
        case SNAKE_LEFT:
            pos.x = state->head->pos.x - 1;
            pos.y = state->head->pos.y;
            break;
        case SNAKE_RIGHT:
            pos.x = state->head->pos.x + 1;
            pos.y = state->head->pos.y;
            break;
    }
    LOG("x = %d; y = %d; dir = %d\n", pos.x, pos.y, state->dir);
    return pos;
}

void move_snake(SnakeState *state, position pos) {
    struct snake_seg *new_head = state->tail;
    new_head->pos = pos;
    SnakeSeg *new_tail = state->tail->prev;
    state->tail = new_tail;
    new_head->prev = NULL;
    state->head->prev = new_head;
    state->head = new_head;
    return;
}

bool overlaps_snake(SnakeState *state, position pos) {
    struct snake_seg *track = state->tail;
    while (track != NULL) {
        if(pos.x == track->pos.x && pos.y == track->pos.y) {
            return true;
        }
        track = track->prev;
    }
    return false;
}

void render_snake(SDL_Renderer *renderer, int x, int y, SnakeState *state) {
    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 255);
    int seg_size = GRID_DIM / GRID_SIZE;
    SDL_Rect seg;
    seg.w = seg_size;
    seg.h = seg_size;

    struct snake_seg *track = state->tail;

    int i = 0;
    while (track != NULL && i < GRID_SIZE * GRID_SIZE) {
        LOG("track = %p;\n", track);
        seg.x = x + track->pos.x * seg_size;
        seg.y = y + track->pos.y * seg_size;

        SDL_RenderFillRect(renderer, &seg);

        track = track->prev;
        i++;
    }
    return;
}

// == Apple ==

typedef struct {
    position pos;
} apple;

apple Apple;

void gen_apple(SnakeState *state, int x, int y, int size) {
    // Try to generate new apple position. If new position overlaps with the
    // snake body, then divide current area into 4 sectors and determine which 
    // of them contains snake's tail. This sector becomes new area for random
    // apple position. Repeat recursively until new position can be found.
    position pos = {x + rand() % size, y + rand() % size};
    LOG("gen_apple: x = %d; y = %d; size = %d\n", x, y, size);
    LOG("gen_apple: apple.x = %d; apple.y = %d\n", pos.x, pos.y);

    if(overlaps_snake(state, pos)) {
        int new_size = size / 2;
        for(int i = 0; i < 2; i++) {
            for(int j = 0; j < 2; j++) {
                int x_start = x + i * new_size;
                int x_end =  x_start + new_size;
                int y_start = y + j * new_size;
                int y_end = y_start + new_size;

                if(state->tail->pos.x < x_end && state->tail->pos.y < y_end) {
                    return gen_apple(state, x_start, y_start, new_size);
                }
            }
        }
    }
    
    Apple.pos = pos;
}

bool overlaps_apple(position pos) {
    return pos.x == Apple.pos.x && pos.y == Apple.pos.y;
}

void render_apple(SDL_Renderer *renderer, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 255);

    int apple_size = GRID_DIM / GRID_SIZE;

    SDL_Rect app;
    app.w = apple_size;
    app.h = apple_size;
    app.x = x + Apple.pos.x * apple_size;
    app.y = y + Apple.pos.y * apple_size;

    SDL_RenderFillRect(renderer, &app);
}

// == Stage ==

bool overlaps_wall(position pos) {
    return pos.x < 0 || pos.x >= GRID_SIZE || pos.y < 0 || pos.y >= GRID_SIZE;
}

void render_grid(SDL_Renderer *renderer, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 0x55, 0x55, 0x55, 0x55);
    int cell_size = GRID_DIM / GRID_SIZE;

    SDL_Rect cell;
    cell.w = cell_size;
    cell.h = cell_size;

    for(int i = 0; i < GRID_SIZE; i++) {
        for(int j = 0; j < GRID_SIZE; j++) {
            cell.x = x + (i * cell_size);
            cell.y = y + (j * cell_size);
            SDL_RenderDrawRect(renderer, &cell);
        }
    }
    return;
}

// == SDL ==

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} SDLH;

SDLH init_sdl() {
    SDL_Window *window;
    SDL_Renderer *renderer;

    if(SDL_INIT_VIDEO < 0) {
        fprintf(stderr, "ERROR: SDL_INIT_VIDEO");
    }

    window = SDL_CreateWindow(
        "Snake",
        WINDOW_X,
        WINDOW_Y,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_BORDERLESS
    );

    if(!window) {
        fprintf(stderr, "ERROR: !window");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        fprintf(stderr, "!renderer");
    }

    SDLH sdlh = {window, renderer};
    return sdlh;
}

void destroy_sdl (SDL_Window *window, SDL_Renderer *renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return;
}

int main(void){
    srand(time(0));

    int max_score = 0;
    GameState game = {score: 0};

    SnakeState *state = init_snake();
    increase_snake(state, next_pos(state));
    
    gen_apple(state, 0, 0, GRID_SIZE);
    game.score += GRID_SIZE * GRID_SIZE;

    SDLH sdlh = init_sdl();
    SDL_Window *window = sdlh.window;
    SDL_Renderer *renderer = sdlh.renderer;

    int grid_x = (WINDOW_WIDTH / 2) - (GRID_DIM / 2);
    int grid_y = (WINDOW_HEIGHT / 2) - (GRID_DIM / 2);

    bool quit = false;
    SDL_Event event;

    while(!quit) {
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYUP:
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            quit = true;
                            break;
                        case SDLK_UP:
                            if(state->dir != SNAKE_DOWN)
                                state->dir = SNAKE_UP;
                            break;
                        case SDLK_DOWN:
                            if(state->dir != SNAKE_UP)
                                state->dir = SNAKE_DOWN;
                            break;
                        case SDLK_LEFT:
                            if(state->dir != SNAKE_RIGHT)
                                state->dir = SNAKE_LEFT;
                            break;
                        case SDLK_RIGHT:
                            if(state->dir != SNAKE_LEFT)
                                state->dir = SNAKE_RIGHT;
                            break;
                    }
                    break;
            }
        }

        // Render loop start
        SDL_RenderClear(renderer);

        position pos = next_pos(state);

        if(overlaps_wall(pos) || overlaps_snake(state, pos)) {
            drop_snake(state);
            state = init_snake();
            increase_snake(state, next_pos(state));
            game.score = GRID_SIZE * GRID_SIZE;
        } else {
            if(overlaps_apple(pos)) {
                increase_snake(state, pos);
                gen_apple(state, 0, 0, GRID_SIZE);
                game.score += GRID_SIZE * GRID_SIZE;
            }
            move_snake(state, pos);
            game.score -= 1;
        }

        if(game.score > max_score) {
            max_score = game.score;
            printf("game: max score = %d\n", game.score);
        }

        render_grid(renderer, grid_x, grid_y);
        render_apple(renderer, grid_x, grid_y);
        render_snake(renderer, grid_x, grid_y, state);
        // Render loop end
        SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, 255);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }
    
    destroy_sdl(window, renderer);
    return 0;
}