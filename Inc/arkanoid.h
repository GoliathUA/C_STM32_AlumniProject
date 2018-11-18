#include "app_util.h"

#define ARKANOID_BUTTON_LEFT BUTTON_LEFT_Pin
#define ARKANOID_BUTTON_RIGHT BUTTON_RIGHT_Pin

#if !defined(ARKANOID_BUTTON_LEFT) || !defined(ARKANOID_BUTTON_RIGHT)
#error Please setup buttons in arkanoid.h, lines 5-6
#endif

#define ARKANOID_GAME_STATUS_PLAYING 1
#define ARKANOID_GAME_STATUS_WIN 2
#define ARKANOID_GAME_STATUS_LOSE 3

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t radius;
    uint16_t color;

    int16_t velocity_x;
    int16_t velocity_y;

    int16_t direction_x;
    int16_t direction_y;

    int16_t previous_x;
    int16_t previous_y;
} ARK_BallTypeDef;

typedef struct
{
    int16_t x;
    int16_t y;
    uint16_t color;

    int16_t velocity_x;
    int16_t velocity_y;

    int16_t previous_x;
    int16_t previous_y;

    uint16_t width;
    uint16_t height;

    int16_t score;
} ARK_PlayerTypeDef;

typedef struct
{
    int16_t x;
    int16_t y;
    uint16_t color;

    uint16_t width;
    uint16_t height;

    uint8_t is_destroying;
    uint8_t is_alive;
    uint16_t score;
    uint8_t index;
    uint8_t quad_tree_index;
} ARK_BlockTypeDef;

typedef struct
{

    uint16_t width;
    uint16_t height;

    uint16_t background_color;

    uint8_t blocks_columns;
    uint8_t blocks_rows;
    uint8_t blocks_size;

    uint8_t status;
    uint8_t is_clean_ball;
    uint8_t is_clean_player;

    APP_RenderingEngineTypeDef *display;

} ARK_SceneStateTypeDef;

volatile ARK_SceneStateTypeDef ARK_Scene;

///////////////////
/// Private Variables
///////////////////

volatile ARK_BallTypeDef __ARK_Ball;
volatile ARK_PlayerTypeDef __ARK_Player;
volatile ARK_BlockTypeDef* __ARK_Blocks;

void Arkanoid_Init(APP_RenderingEngineTypeDef *display);

void Arkanoid_Draw();
void Arkanoid_HandleRusult();
void Arkanoid_HandleIO(uint16_t GPIO_Pin);

///////////////////
/// DRAW
///////////////////

void __Arkanoid_Draw_Ball();
void __Arkanoid_Draw_Player();
void __Arkanoid_Draw_Blocks();

///////////////////
/// Physics
///////////////////

void Arkanoid_WorldUpdate();
void __Arkanoid_Physic_ColissionDetection();
uint8_t __Arkanoid_Physic_HasCollision(
    int16_t x,
    int16_t y,
    int16_t bound_x,
    int16_t bound_y,
    int16_t target_x,
    int16_t target_y,
    int16_t target_bound_x,
    int16_t target_bound_y);
void __Arkanoid_Physic_MoveBall();
void __Arkanoid_Physic_MovePlayerLeft();
void __Arkanoid_Physic_MovePlayerRight();

