#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <raymath.h>

#define BIT_MAP_WIDTH 10
#define BIT_MAP_HEIGHT 10

typedef struct
{
    int *map;
    int tile_count_width, tile_count_height;
    
    // based on the screen dimensions
    int tile_size;
    int width_px, height_px;
    int offset_x, offset_y;
}BitMap;

typedef struct 
{
    Vector2 pos;
    Vector2 direction;
}Player;

static inline int accessBitMap(BitMap *bit_map, int x, int y) {
    return bit_map->map[bit_map->tile_count_width * y + x];
}

static inline int writeBitMap(BitMap *bit_map, int x, int y, int val) {
    return bit_map->map[bit_map->tile_count_width * y + x] = val;
}

static inline void printBitMap(BitMap *bit_map) {
    for (int y = 0; y < bit_map->tile_count_height; y++)
    {
        for (int x = 0; x < bit_map->tile_count_width; x++)
        {
            if (accessBitMap(bit_map, x, y) != 0)
            {
                printf("1 ");
            }
            else
            {
                printf("0 ");
            }
        }
        printf("\n");
    }
}

void updateBitMap(BitMap *map, int screen_width, int screen_height)
{
    if (screen_width > screen_height)
    {
        map->tile_size = screen_height / map->tile_count_height;
    }else
    {
        map->tile_size = screen_width / map->tile_count_width;
    }

    // calculate the bit map size
    map->width_px  = map->tile_count_width  * map->tile_size;
    map->height_px = map->tile_count_height * map->tile_size;

    // calculate the offset where the map is placed
    map->offset_x = screen_width  / 2 - map->width_px  / 2;
    map->offset_y = screen_height / 2 - map->height_px / 2;
}

#define SIDE_OFFSET 2

void drawBitMap(BitMap *map)
{
    // Draw horizontal lines
    for (int y = map->offset_y + map->tile_size; y < map->width_px + map->offset_y; y += map->tile_size)
    {
        DrawLine(0 + map->offset_x, y, map->width_px + map->offset_x, y, BLACK);
    }

    // Draw vertical lines
    for (int x = map->offset_x + map->tile_size; x < map->width_px + map->offset_x; x += map->tile_size)
    {
        DrawLine(x, 0 + map->offset_y, x, map->height_px + map->offset_y, BLACK);
    }

    // Draw in the bitmap
    for (int y = 0; y < map->tile_count_height; y++)
    {
        for (int x = 0; x < map->tile_count_width; x++)
        {
            if (accessBitMap(map, x, y) != 0)
            {
                // i dont know why but we need to subtract 1 from the side offset when considering the rect width and height
                DrawRectangle(x * map->tile_size + map->offset_x + SIDE_OFFSET, y * map->tile_size + map->offset_y + SIDE_OFFSET, map->tile_size - SIDE_OFFSET * 2, map->tile_size - SIDE_OFFSET * 2, BLACK);
            }
        }
    }
}

int *getMousePos(BitMap *map)
{
    Vector2 mouse = GetMousePosition();

    mouse.x -= map->offset_x;
    mouse.y -= map->offset_y;
    
    mouse.x = (int)(mouse.x / map->tile_size);
    mouse.y = (int)(mouse.y / map->tile_size);
    
    mouse.x = mouse.x > map->tile_count_width  - 1 ? map->tile_count_width  - 1 : mouse.x;
    mouse.y = mouse.y > map->tile_count_height - 1 ? map->tile_count_height - 1 : mouse.y;
    mouse.x = mouse.x < 0 ? 0 : mouse.x;
    mouse.y = mouse.y < 0 ? 0 : mouse.y;

    return &map->map[(int)mouse.x + (int)mouse.y * map->tile_count_width];
}

#define LINE_LEN 20.0f
void drawPlayer(Player *player)
{
    // draw players point
    DrawCircleV(player->pos, 3, DARKBLUE);
    
    DrawLineV(player->pos, Vector2Add(Vector2Scale(player->direction, LINE_LEN), player->pos), DARKBLUE);
}

void movePlayer(Player *player)
{
    if(IsKeyDown(KEY_W))
    {
        player->pos = Vector2Add(player->pos, Vector2Scale(player->direction, 3));
    }
    if(IsKeyDown(KEY_S))
    {
        player->pos = Vector2Subtract(player->pos, Vector2Scale(player->direction, 3));
    }
    if(IsKeyDown(KEY_A))
    {
        player->direction = Vector2Rotate(player->direction, 0.1);
    }
    if(IsKeyDown(KEY_D))
    {
        player->direction = Vector2Rotate(player->direction, -0.1);
    }
}

int main ()
{
    int screen_width  = 800;
    int screen_height = 650;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screen_width, screen_height, "Raycaster Example");
    SetTargetFPS(60);

    // init the bit map
    int bit_map[BIT_MAP_WIDTH * BIT_MAP_HEIGHT] = {0};
    BitMap map;
    map.tile_count_width  = BIT_MAP_WIDTH;
    map.tile_count_height = BIT_MAP_HEIGHT;
    map.map               = (int *)bit_map;

    // init the player
    Player player;
    player.pos = (Vector2){300, 300};
    player.direction = Vector2Normalize((Vector2){1, 0.75});

    while (!WindowShouldClose())
    {
        BeginDrawing();
        screen_width = GetScreenWidth();
        screen_height = GetScreenHeight();
        ClearBackground(RAYWHITE);

        updateBitMap(&map, screen_width, screen_height);
        drawBitMap(&map);

        if (IsKeyPressed(KEY_C))
        {
            memset(map.map, 0, map.tile_count_width * map.tile_count_height * sizeof(int));
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            *getMousePos(&map) = 1;
        }else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            *getMousePos(&map) = 0;
        }

        movePlayer(&player);
        drawPlayer(&player);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}