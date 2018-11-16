#include "arkanoid.h"

void Arkanoid_Init(APP_RenderingEngineTypeDef *draw_engine)
{
    ark_scene.draw_engine = draw_engine;

    // Scene
    ark_scene.background_color = ark_scene.draw_engine->backgroundColor;
    ark_scene.width = ark_scene.draw_engine->screenWidth;
    ark_scene.height = ark_scene.draw_engine->screenHeight;
    ark_scene.status = ARKANOID_GAME_STATUS_PLAYING;
    ark_scene.blocks_columns = 3;
    ark_scene.blocks_rows = 3;

    // Blocks
    ark_scene.blocks_size = ark_scene.blocks_columns * ark_scene.blocks_rows;

    ark_blocks = (ARK_BlockTypeDef*) calloc(
        ark_scene.blocks_size,
        sizeof(ARK_BlockTypeDef));

    uint16_t current_row_width = 0;
    uint16_t current_column_index = 0;
    uint16_t current_y = 15;
    uint16_t bound_size = 10;
    for (int i = 0; i < ark_scene.blocks_size; i++) {
        ark_blocks[i].is_alive = 1;
        ark_blocks[i].color = ark_scene.draw_engine->color(
            255,
            255,
            (i + 25) * (current_column_index + 25));
        ark_blocks[i].height = 15;
        ark_blocks[i].width = (ark_scene.width / ark_scene.blocks_columns) - bound_size;
        ark_blocks[i].x = current_column_index * (ark_blocks[i].width + bound_size);
        ark_blocks[i].y = current_y;
        ark_blocks[i].score = 10;

        current_column_index++;
        current_row_width += ark_blocks[i].width + bound_size;
        if (current_row_width >= ark_scene.width) {
            current_column_index = 0;
            current_row_width = 0;
            current_y += ark_blocks[i].height + 5;
        }
    }

    //malloc(sizeof(int) * vector->capacity)

    // Ball
    ark_ball.x = ark_scene.width / 2;
    ark_ball.y = ark_scene.height / 2;
    ark_ball.radius = 5;
    ark_ball.color = ark_scene.draw_engine->color(255, 255, 255);

    ark_ball.velocity_x = 1;
    ark_ball.velocity_y = 1;

    ark_ball.direction_x = -1;
    ark_ball.direction_y = -1;

    ark_ball.previous_x = ark_ball.x;
    ark_ball.previous_y = ark_ball.y;

    // Player
    ark_player.width = 100;
    ark_player.height = 10;
    ark_player.x = (ark_scene.width / 2) - (ark_player.width / 2);
    ark_player.y = ark_scene.height - ark_player.height;
    ark_player.previous_x = ark_player.x;
    ark_player.previous_y = ark_player.y;
    ark_player.color = ark_scene.draw_engine->color(255, 0, 0);
    ark_player.score = 0;
}

void Arkanoid_Draw()
{
    if (ark_scene.status == ARKANOID_GAME_STATUS_PLAYING) {
        ark_scene.draw_engine->cursor(0, 0);
        ark_scene.draw_engine->printf("Score: %d", ark_player.score);
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

    if (ark_scene.is_clean_ball) {
        ark_scene.is_clean_ball = 0;
        ark_scene.draw_engine->circle(
            ark_ball.previous_x,
            ark_ball.previous_y,
            ark_ball.radius + 2,
            ark_scene.background_color);
    }

    ark_scene.draw_engine->circle(ark_ball.x, ark_ball.y, ark_ball.radius, ark_ball.color);
}

void __Arkanoid_Draw_Blocks()
{
    for (int i = 0; i < ark_scene.blocks_size; i++) {

        if (ark_blocks[i].is_destroying) {
            ark_blocks[i].is_destroying = 0;
            ark_scene.draw_engine->rect(
                ark_blocks[i].x,
                ark_blocks[i].y,
                ark_blocks[i].width,
                ark_blocks[i].height,
                ark_scene.background_color);
        }

        if (!ark_blocks[i].is_alive) {
            continue;
        }
        ark_scene.draw_engine->rect(
            ark_blocks[i].x,
            ark_blocks[i].y,
            ark_blocks[i].width,
            ark_blocks[i].height,
            ark_blocks[i].color);
    }
}

void __Arkanoid_Draw_Player()
{

    if (ark_scene.is_clean_player) {
        ark_scene.is_clean_player = 0;
        ark_scene.draw_engine->rect(
            0,
            ark_player.previous_y,
            ark_scene.width,
            ark_player.height,
            ark_scene.background_color);

    }

    ark_scene.draw_engine->rect(
        ark_player.x,
        ark_player.y,
        ark_player.width,
        ark_player.height,
        ark_player.color);

}

void __Arkanoid_Physic_MovePlayerLeft()
{
    ark_scene.is_clean_player = 1;

    ark_player.previous_x = ark_player.x;
    ark_player.previous_y = ark_player.y;

    ark_player.x -= 10;

    if (ark_player.x <= 0) {
        ark_player.x = 0;
    }
}

void __Arkanoid_Physic_MovePlayerRight()
{
    ark_scene.is_clean_player = 1;

    ark_player.previous_x = ark_player.x;
    ark_player.previous_y = ark_player.y;

    ark_player.x += 10;

    if ((ark_player.x + ark_player.width) >= ark_scene.width) {
        ark_player.x = ark_scene.width - ark_player.width;
    }

}

void __Arkanoid_Physic_ColissionDetection()
{
    int16_t x = ark_ball.x;
    int16_t y = ark_ball.y;
    int16_t bound_x = ark_ball.radius / 2;
    int16_t bound_y = ark_ball.radius / 2;

    int16_t target_x = ark_player.x + (ark_player.width / 2);
    int16_t target_y = ark_player.y;
    int16_t target_bound_x = ark_player.width / 2;
    int16_t target_bound_y = ark_player.height / 2;

    if (__Arkanoid_Physic_HasCollision(
        x,
        y,
        bound_x,
        bound_y,
        target_x,
        target_y,
        target_bound_x,
        target_bound_y)) {
        ark_ball.direction_y *= -1;
    } else {
        for (int i = 0; i < ark_scene.blocks_size; i++) {
            if (!ark_blocks[i].is_alive) {
                continue;
            }

            target_bound_x = ark_blocks[i].width / 2;
            target_bound_y = ark_blocks[i].height / 2;
            target_x = ark_blocks[i].x + target_bound_x;
            target_y = ark_blocks[i].y + target_bound_y;
            if (__Arkanoid_Physic_HasCollision(
                x,
                y,
                bound_x,
                bound_y,
                target_x,
                target_y,
                target_bound_x,
                target_bound_y)) {
                ark_blocks[i].is_destroying = 1;
                ark_blocks[i].is_alive = 0;
                ark_ball.direction_y *= -1;
                ark_player.score += ark_blocks[i].score;
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

    ark_scene.is_clean_ball = 1;

    ark_ball.previous_x = ark_ball.x;
    ark_ball.previous_y = ark_ball.y;

    if (ark_ball.x >= ark_scene.width || ark_ball.x <= 0) {
        ark_ball.direction_x *= -1;
    } else if (ark_ball.y <= 0) {
        ark_ball.direction_y *= -1;
    } else if (ark_ball.y >= ark_scene.height) {
        ark_scene.status = ARKANOID_GAME_STATUS_LOSE;
        //ball.direction_y *= -1;
    }

    ark_ball.x += ark_ball.velocity_x * ark_ball.direction_x;
    ark_ball.y += ark_ball.velocity_y * ark_ball.direction_y;

}

void Arkanoid_WorldUpdate()
{

    if (ark_scene.status != ARKANOID_GAME_STATUS_PLAYING) {
        return;
    }

    __Arkanoid_Physic_MoveBall();
    __Arkanoid_Physic_ColissionDetection();

    uint8_t has_alive_blocks = 0;
    for (int i = 0; i < ark_scene.blocks_size; i++) {
        if (ark_blocks[i].is_alive) {
            has_alive_blocks = 1;
            break;
        }
    }

    if (!has_alive_blocks) {
        ark_scene.status = ARKANOID_GAME_STATUS_WIN;
    }
}

__weak void Arkanoid_HandleRusult()
{
    if (ark_scene.status == ARKANOID_GAME_STATUS_WIN) {
        ark_scene.draw_engine->cursor((ark_scene.width / 2) - 40, ark_scene.height / 2);
        ark_scene.draw_engine->background(ark_scene.draw_engine->backgroundColor);
        ark_scene.draw_engine->printf("WIN!");
    } else {
        ark_scene.draw_engine->background(ark_scene.draw_engine->backgroundColor);
        ark_scene.draw_engine->cursor((ark_scene.width / 2) - 10, ark_scene.height / 2);
        ark_scene.draw_engine->printf("Game Over!");
    }
}
