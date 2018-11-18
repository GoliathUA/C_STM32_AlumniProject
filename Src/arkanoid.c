#include "arkanoid.h"

void Arkanoid_Init(APP_RenderingEngineTypeDef *display)
{
    ARK_Scene.display = display;

    // Scene
    ARK_Scene.background_color = ARK_Scene.display->backgroundColor;
    ARK_Scene.width = ARK_Scene.display->screenWidth;
    ARK_Scene.height = ARK_Scene.display->screenHeight;
    ARK_Scene.status = ARKANOID_GAME_STATUS_PLAYING;
    ARK_Scene.blocks_columns = 3;
    ARK_Scene.blocks_rows = 3;

    // Blocks
    ARK_Scene.blocks_size = ARK_Scene.blocks_columns * ARK_Scene.blocks_rows;

    __ARK_Blocks = (ARK_BlockTypeDef*) calloc(
        ARK_Scene.blocks_size,
        sizeof(ARK_BlockTypeDef));

    uint16_t current_row_width = 0;
    uint16_t current_column_index = 0;
    uint16_t current_y = 15;
    uint16_t bound_size = 10;
    for (int i = 0; i < ARK_Scene.blocks_size; i++) {
        __ARK_Blocks[i].is_alive = 1;
        __ARK_Blocks[i].color = ARK_Scene.display->color(
            255,
            255,
            (i + 25) * (current_column_index + 25));
        __ARK_Blocks[i].height = 15;
        __ARK_Blocks[i].width = (ARK_Scene.width / ARK_Scene.blocks_columns)
                - bound_size;
        __ARK_Blocks[i].x = current_column_index
                * (__ARK_Blocks[i].width + bound_size);
        __ARK_Blocks[i].y = current_y;
        __ARK_Blocks[i].score = 10;

        current_column_index++;
        current_row_width += __ARK_Blocks[i].width + bound_size;
        if (current_row_width >= ARK_Scene.width) {
            current_column_index = 0;
            current_row_width = 0;
            current_y += __ARK_Blocks[i].height + 5;
        }
    }

    //malloc(sizeof(int) * vector->capacity)

    // Ball
    __ARK_Ball.x = ARK_Scene.width / 2;
    __ARK_Ball.y = ARK_Scene.height / 2;
    __ARK_Ball.radius = 5;
    __ARK_Ball.color = ARK_Scene.display->color(255, 255, 255);

    __ARK_Ball.velocity_x = 1;
    __ARK_Ball.velocity_y = 1;

    __ARK_Ball.direction_x = -1;
    __ARK_Ball.direction_y = -1;

    __ARK_Ball.previous_x = __ARK_Ball.x;
    __ARK_Ball.previous_y = __ARK_Ball.y;

    // Player
    __ARK_Player.width = 100;
    __ARK_Player.height = 10;
    __ARK_Player.x = (ARK_Scene.width / 2) - (__ARK_Player.width / 2);
    __ARK_Player.y = ARK_Scene.height - __ARK_Player.height;
    __ARK_Player.previous_x = __ARK_Player.x;
    __ARK_Player.previous_y = __ARK_Player.y;
    __ARK_Player.color = ARK_Scene.display->color(255, 0, 0);
    __ARK_Player.score = 0;
}

void Arkanoid_Draw()
{
    if (ARK_Scene.status == ARKANOID_GAME_STATUS_PLAYING) {
        ARK_Scene.display->cursor(0, 0);
        ARK_Scene.display->printf("Score: %d", __ARK_Player.score);
        __Arkanoid_Draw_Ball();
        __Arkanoid_Draw_Player();
        __Arkanoid_Draw_Blocks();

    } else {
        Arkanoid_HandleRusult();
    }
}

void Arkanoid_HandleIO(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ARKANOID_BUTTON_LEFT) {
        __Arkanoid_Physic_MovePlayerLeft();
    } else if (GPIO_Pin == ARKANOID_BUTTON_RIGHT) {
        __Arkanoid_Physic_MovePlayerRight();
    }
}

void __Arkanoid_Draw_Ball()
{

    if (ARK_Scene.is_clean_ball) {
        ARK_Scene.is_clean_ball = 0;
        ARK_Scene.display->circle(
            __ARK_Ball.previous_x,
            __ARK_Ball.previous_y,
            __ARK_Ball.radius + 2,
            ARK_Scene.background_color);
    }

    ARK_Scene.display->circle(
        __ARK_Ball.x,
        __ARK_Ball.y,
        __ARK_Ball.radius,
        __ARK_Ball.color);
}

void __Arkanoid_Draw_Blocks()
{
    for (int i = 0; i < ARK_Scene.blocks_size; i++) {

        if (__ARK_Blocks[i].is_destroying) {
            __ARK_Blocks[i].is_destroying = 0;
            ARK_Scene.display->rect(
                __ARK_Blocks[i].x,
                __ARK_Blocks[i].y,
                __ARK_Blocks[i].width,
                __ARK_Blocks[i].height,
                ARK_Scene.background_color);
        }

        if (!__ARK_Blocks[i].is_alive) {
            continue;
        }
        ARK_Scene.display->rect(
            __ARK_Blocks[i].x,
            __ARK_Blocks[i].y,
            __ARK_Blocks[i].width,
            __ARK_Blocks[i].height,
            __ARK_Blocks[i].color);
    }
}

void __Arkanoid_Draw_Player()
{

    if (ARK_Scene.is_clean_player) {
        ARK_Scene.is_clean_player = 0;
        ARK_Scene.display->rect(
            0,
            __ARK_Player.previous_y,
            ARK_Scene.width,
            __ARK_Player.height,
            ARK_Scene.background_color);

    }

    ARK_Scene.display->rect(
        __ARK_Player.x,
        __ARK_Player.y,
        __ARK_Player.width,
        __ARK_Player.height,
        __ARK_Player.color);

}

void __Arkanoid_Physic_MovePlayerLeft()
{
    ARK_Scene.is_clean_player = 1;

    __ARK_Player.previous_x = __ARK_Player.x;
    __ARK_Player.previous_y = __ARK_Player.y;

    __ARK_Player.x -= 10;

    if (__ARK_Player.x <= 0) {
        __ARK_Player.x = 0;
    }
}

void __Arkanoid_Physic_MovePlayerRight()
{
    ARK_Scene.is_clean_player = 1;

    __ARK_Player.previous_x = __ARK_Player.x;
    __ARK_Player.previous_y = __ARK_Player.y;

    __ARK_Player.x += 10;

    if ((__ARK_Player.x + __ARK_Player.width) >= ARK_Scene.width) {
        __ARK_Player.x = ARK_Scene.width - __ARK_Player.width;
    }

}

void __Arkanoid_Physic_ColissionDetection()
{
    int16_t x = __ARK_Ball.x;
    int16_t y = __ARK_Ball.y;
    int16_t bound_x = __ARK_Ball.radius / 2;
    int16_t bound_y = __ARK_Ball.radius / 2;

    int16_t target_x = __ARK_Player.x + (__ARK_Player.width / 2);
    int16_t target_y = __ARK_Player.y;
    int16_t target_bound_x = __ARK_Player.width / 2;
    int16_t target_bound_y = __ARK_Player.height / 2;

    if (__Arkanoid_Physic_HasCollision(
        x,
        y,
        bound_x,
        bound_y,
        target_x,
        target_y,
        target_bound_x,
        target_bound_y)) {
        __ARK_Ball.direction_y *= -1;
    } else {
        for (int i = 0; i < ARK_Scene.blocks_size; i++) {
            if (!__ARK_Blocks[i].is_alive) {
                continue;
            }

            target_bound_x = __ARK_Blocks[i].width / 2;
            target_bound_y = __ARK_Blocks[i].height / 2;
            target_x = __ARK_Blocks[i].x + target_bound_x;
            target_y = __ARK_Blocks[i].y + target_bound_y;
            if (__Arkanoid_Physic_HasCollision(
                x,
                y,
                bound_x,
                bound_y,
                target_x,
                target_y,
                target_bound_x,
                target_bound_y)) {
                __ARK_Blocks[i].is_destroying = 1;
                __ARK_Blocks[i].is_alive = 0;
                __ARK_Ball.direction_y *= -1;
                __ARK_Player.score += __ARK_Blocks[i].score;
            }
        }
    }

}

uint8_t __Arkanoid_Physic_HasCollision(
    int16_t x,
    int16_t y,
    int16_t bound_x,
    int16_t bound_y,
    int16_t target_x,
    int16_t target_y,
    int16_t target_bound_x,
    int16_t target_bound_y)
{

    return (x + bound_x > target_x - target_bound_x)
            && (x - bound_x < target_x + target_bound_x)
            && (y + bound_y > target_y - target_bound_y)
            && (y - bound_y < target_y + target_bound_y);
}

void __Arkanoid_Physic_MoveBall()
{

    ARK_Scene.is_clean_ball = 1;

    __ARK_Ball.previous_x = __ARK_Ball.x;
    __ARK_Ball.previous_y = __ARK_Ball.y;

    if (__ARK_Ball.x >= ARK_Scene.width || __ARK_Ball.x <= 0) {
        __ARK_Ball.direction_x *= -1;
    } else if (__ARK_Ball.y <= 0) {
        __ARK_Ball.direction_y *= -1;
    } else if (__ARK_Ball.y >= ARK_Scene.height) {
        ARK_Scene.status = ARKANOID_GAME_STATUS_LOSE;
        //ball.direction_y *= -1;
    }

    __ARK_Ball.x += __ARK_Ball.velocity_x * __ARK_Ball.direction_x;
    __ARK_Ball.y += __ARK_Ball.velocity_y * __ARK_Ball.direction_y;

}

void Arkanoid_WorldUpdate()
{

    if (ARK_Scene.status != ARKANOID_GAME_STATUS_PLAYING) {
        return;
    }

    __Arkanoid_Physic_MoveBall();
    __Arkanoid_Physic_ColissionDetection();

    uint8_t has_alive_blocks = 0;
    for (int i = 0; i < ARK_Scene.blocks_size; i++) {
        if (__ARK_Blocks[i].is_alive) {
            has_alive_blocks = 1;
            break;
        }
    }

    if (!has_alive_blocks) {
        ARK_Scene.status = ARKANOID_GAME_STATUS_WIN;
    }
}

__weak void Arkanoid_HandleRusult()
{
    if (ARK_Scene.status == ARKANOID_GAME_STATUS_WIN) {
        ARK_Scene.display->cursor((ARK_Scene.width / 2) - 40, ARK_Scene.height / 2);
        ARK_Scene.display->background(ARK_Scene.display->backgroundColor);
        ARK_Scene.display->printf("WIN!");
    } else {
        ARK_Scene.display->background(ARK_Scene.display->backgroundColor);
        ARK_Scene.display->cursor((ARK_Scene.width / 2) - 10, ARK_Scene.height / 2);
        ARK_Scene.display->printf("Game Over!");
    }
}
