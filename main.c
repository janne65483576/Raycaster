#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <raymath.h>
#include <stdlib.h>

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
    Vector2 position;
    Vector2 plane;
    Vector2 direction;
}Player;

typedef struct
{
    int x, y;
} BitMapCoord;

typedef struct 
{
    Vector2 position;
    Texture2D texture;
    float speed;
} Sprite;

typedef struct {
    float distance;
    int hit_side;
    bool hit_tile;
} RayHit;

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

BitMapCoord screenToBitMap(BitMap *map, Vector2 input_pos)
{
    BitMapCoord res;
    
    res.x = ((int)input_pos.x - map->offset_x) / map->tile_size;
    res.y = ((int)input_pos.y - map->offset_y) / map->tile_size;

    if (res.x > map->tile_count_width - 1)
    {
        res.x = map->tile_count_width - 1;
    }
    else if (res.x < 0)
    {
        res.x = 0;
    }

    if (res.y > map->tile_count_height - 1)
    {
        res.y = map->tile_count_height - 1;
    }
    else if (res.y < 0) 
    {
        res.y = 0;
    }

    return res;
}

int *getMousePos(BitMap *map)
{
    BitMapCoord coord =  screenToBitMap(map, GetMousePosition());

    return &map->map[coord.x + coord.y * map->tile_count_width];
}

#define LINE_LEN 20.0f
void drawPlayer(Player *player)
{
    // draw players point
    DrawCircleV(player->position, 3, DARKBLUE);
    
    // draw a line pointing in player direction
    DrawLineV(player->position, Vector2Add(Vector2Scale(player->direction, LINE_LEN), player->position), DARKBLUE);
}

#define PLAYER_MOVE_SPEED   50.0f
#define PLAYER_ROTATE_SPEED 1.0f

void movePlayer(BitMap *map, Player *player, double fov)
{
    float delta_time = GetFrameTime();

    if(IsKeyDown(KEY_W))
    {
        Vector2 new_pos = Vector2Add(player->position, Vector2Scale(player->direction, PLAYER_MOVE_SPEED * delta_time));

        BitMapCoord bit_map_coord = screenToBitMap(map, new_pos);

        // check if the new position is valid
        if (accessBitMap(map, bit_map_coord.x, bit_map_coord.y) == 0)
        {
            player->position = new_pos;
        }
    }
    if(IsKeyDown(KEY_S))
    {
        Vector2 new_pos = Vector2Subtract(player->position, Vector2Scale(player->direction, PLAYER_MOVE_SPEED * delta_time));

        BitMapCoord bit_map_coord = screenToBitMap(map, new_pos);

        // check if the new position is valid
        if (accessBitMap(map, bit_map_coord.x, bit_map_coord.y) == 0)
        {
            player->position = new_pos;
        }
    }
    if(IsKeyDown(KEY_A))
    {
        player->direction = Vector2Rotate(player->direction, -PLAYER_ROTATE_SPEED * delta_time);
        player->plane = Vector2Scale((Vector2){ -player->direction.y, player->direction.x }, tan(fov / 2.0f));
    }
    if(IsKeyDown(KEY_D))
    {
        player->direction = Vector2Rotate(player->direction, PLAYER_ROTATE_SPEED * delta_time);
        player->plane = Vector2Scale((Vector2){ -player->direction.y, player->direction.x }, tan(fov / 2.0f));
    }
}

RayHit castRay(BitMap *map, Vector2 start_pos, Vector2 direction)
{
    BitMapCoord map_coord = screenToBitMap(map, start_pos);

    double delta_x = (direction.x == 0) ? 1e30 : fabs(1.0 / direction.x);
    double delta_y = (direction.y == 0) ? 1e30 : fabs(1.0 / direction.y);

    double curr_dist_x, curr_dist_y;
    int step_x, step_y;

    if (direction.x < 0)
    {
        curr_dist_x = (start_pos.x - (map_coord.x * map->tile_size + map->offset_x)) / map->tile_size * delta_x;
        step_x = -1;
    } else 
    {
        curr_dist_x = (((map_coord.x + 1) * map->tile_size + map->offset_x) - start_pos.x) / map->tile_size * delta_x;
        step_x = 1;
    }

    if (direction.y < 0)
    {
        curr_dist_y = (start_pos.y - (map_coord.y * map->tile_size + map->offset_y)) / map->tile_size * delta_y;
        step_y = -1;
    } else 
    {
        curr_dist_y = (((map_coord.y + 1) * map->tile_size + map->offset_y) - start_pos.y) / map->tile_size * delta_y;
        step_y = 1;
    }

    int hit_side; // 0 = x, 1 = y

    while(true)
    {
        if (curr_dist_x < curr_dist_y)
        {
            curr_dist_x += delta_x;
            hit_side = 0;
            map_coord.x += step_x;
        } else
        {
            curr_dist_y += delta_y;
            hit_side = 1;
            map_coord.y += step_y;
        }

        if (map_coord.x >= map->tile_count_width  || map_coord.x < 0 || 
            map_coord.y >= map->tile_count_height || map_coord.y < 0)
        {
            return(RayHit){.hit_tile = false};
        }

        if (accessBitMap(map, map_coord.x, map_coord.y) != 0)
        {
            break;
        }
    }

    double ray_length;

    if (hit_side == 0)
    {
        ray_length = curr_dist_x - delta_x;
    } else 
    {
        ray_length = curr_dist_y - delta_y;
    }

    return(RayHit){ray_length, hit_side, .hit_tile = true};
}

void renderScene(BitMap *map, Player *player)
{
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    
    for (int x_coord = 0; x_coord < GetRenderWidth(); x_coord++)
    {
        double camera_x = 2.0f * x_coord / screen_width - 1.0f;

        Vector2 ray_direction = Vector2Add(player->direction, Vector2Scale(player->plane, camera_x));

        RayHit hit = castRay(map, player->position, ray_direction);
        
        if (hit.hit_tile == false) continue;

        int line_height = (int)(screen_height / hit.distance);

        int draw_start = -line_height / 2 + screen_height / 2;
        int draw_end   =  line_height / 2 + screen_height / 2;

        if (draw_start < 0) draw_start = 0;
        if (draw_end >= screen_height) draw_end = screen_height - 1;

        Color wall_color = (hit.hit_side == 0) ? GRAY : DARKGRAY;

        DrawLine(x_coord, draw_start, x_coord, draw_end, wall_color);
    }
}

int main ()
{
    int screen_width  = 800;
    int screen_height = 650;
    
    double fov = 60 * DEG2RAD;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screen_width, screen_height, "Raycaster Example");
    //SetTargetFPS(60);

    // init the bit map
    int bit_map[BIT_MAP_WIDTH * BIT_MAP_HEIGHT] = {0};
    BitMap map;
    map.tile_count_width  = BIT_MAP_WIDTH;
    map.tile_count_height = BIT_MAP_HEIGHT;
    map.map               = (int *)bit_map;

    // init the player
    Player player;
    player.position = (Vector2){300, 300};
    player.direction = (Vector2){1, 1};
    player.plane = Vector2Scale((Vector2){ -player.direction.y, player.direction.x },tan(fov / 2.0f));

    // init sprite
    Sprite enemy = {
        .position = (Vector2){ 450, 300 },
        .texture = LoadTexture("texture.jpeg")
    };

    bool draw_bitmap = true;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        screen_width = GetScreenWidth();
        screen_height = GetScreenHeight();
        ClearBackground(RAYWHITE);

        updateBitMap(&map, screen_width, screen_height);

        if (IsKeyPressed(KEY_C))
        {
            player.position = (Vector2){screen_width / 2, screen_height / 2};
            memset(map.map, 0, map.tile_count_width * map.tile_count_height * sizeof(int));
        }

        if (IsKeyPressed(KEY_M))
        {
            draw_bitmap = !draw_bitmap;
        }
        
        movePlayer(&map, &player, fov);
        
        if (draw_bitmap)
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                *getMousePos(&map) = 1;
            }else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                *getMousePos(&map) = 0;
            }

            drawBitMap(&map);
            drawPlayer(&player);
        }else {
            renderScene(&map, &player);
        }
        
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}