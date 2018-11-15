#include "arkanoid.h"

void Arkanoid_Init(APP_RenderingEngineTypeDef *draw_engine)
{
    scene.draw_engine = draw_engine;

    // Scene
    scene.background_color = scene.draw_engine->backgroundColor;
    scene.width = scene.draw_engine->screenWidth;
    scene.height = scene.draw_engine->screenHeight;
    scene.status = ARKANOID_GAME_STATUS_PLAYING;
    scene.blocks_columns = 3;
    scene.blocks_rows = 3;

    // Blocks
    scene.blocks_size = scene.blocks_columns * scene.blocks_rows;

    blocks = (ARK_BlockTypeDef*) calloc(
        scene.blocks_size,
        sizeof(ARK_BlockTypeDef));

    uint16_t current_row_width = 0;
    uint16_t current_column_index = 0;
    uint16_t current_y = 15;
    uint16_t bound_size = 10;
    for (int i = 0; i < scene.blocks_size; i++) {
        blocks[i].is_alive = 1;
        blocks[i].color = scene.draw_engine->color(
            255,
            255,
            (i + 25) * (current_column_index + 25));
        blocks[i].height = 15;
        blocks[i].width = (scene.width / scene.blocks_columns) - bound_size;
        blocks[i].x = current_column_index * (blocks[i].width + bound_size);
        blocks[i].y = current_y;
        blocks[i].score = 10;

        current_column_index++;
        current_row_width += blocks[i].width + bound_size;
        if (current_row_width >= scene.width) {
            current_column_index = 0;
            current_row_width = 0;
            current_y += blocks[i].height + 5;
        }
    }

    //malloc(sizeof(int) * vector->capacity)

    // Ball
    ball.x = scene.width / 2;
    ball.y = scene.height / 2;
    ball.radius = 5;
    ball.color = scene.draw_engine->color(255, 255, 255);

    ball.velocity_x = 1;
    ball.velocity_y = 1;

    ball.direction_x = -1;
    ball.direction_y = -1;

    ball.previous_x = ball.x;
    ball.previous_y = ball.y;

    // Player
    player.width = 100;
    player.height = 10;
    player.x = (scene.width / 2) - (player.width / 2);
    player.y = scene.height - player.height;
    player.previous_x = player.x;
    player.previous_y = player.y;
    player.color = scene.draw_engine->color(255, 0, 0);
    player.score = 0;
}

void Arkanoid_Draw()
{
    if (scene.status == ARKANOID_GAME_STATUS_PLAYING) {
        scene.draw_engine->cursor(0, 0);
        scene.draw_engine->printf("Score: %d", player.score);
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

    if (scene.is_clean_ball) {
        scene.is_clean_ball = 0;
        scene.draw_engine->circle(
            ball.previous_x,
            ball.previous_y,
            ball.radius + 2,
            scene.background_color);
    }

    scene.draw_engine->circle(ball.x, ball.y, ball.radius, ball.color);
}

void __Arkanoid_Draw_Blocks()
{
    for (int i = 0; i < scene.blocks_size; i++) {

        if (blocks[i].is_destroying) {
            blocks[i].is_destroying = 0;
            scene.draw_engine->rect(
                blocks[i].x,
                blocks[i].y,
                blocks[i].width,
                blocks[i].height,
                scene.background_color);
        }

        if (!blocks[i].is_alive) {
            continue;
        }
        scene.draw_engine->rect(
            blocks[i].x,
            blocks[i].y,
            blocks[i].width,
            blocks[i].height,
            blocks[i].color);
    }
}

void __Arkanoid_Draw_Player()
{

    if (scene.is_clean_player) {
        scene.is_clean_player = 0;
        scene.draw_engine->rect(
            0,
            player.previous_y,
            scene.width,
            player.height,
            scene.background_color);

    }

    scene.draw_engine->rect(
        player.x,
        player.y,
        player.width,
        player.height,
        player.color);

}

void __Arkanoid_Physic_MovePlayerLeft()
{
    scene.is_clean_player = 1;

    player.previous_x = player.x;
    player.previous_y = player.y;

    player.x -= 10;

    if (player.x <= 0) {
        player.x = 0;
    }
}

void __Arkanoid_Physic_MovePlayerRight()
{
    scene.is_clean_player = 1;

    player.previous_x = player.x;
    player.previous_y = player.y;

    player.x += 10;

    if ((player.x + player.width) >= scene.width) {
        player.x = scene.width - player.width;
    }

}

void __Arkanoid_Physic_ColissionDetection()
{
    int16_t x = ball.x;
    int16_t y = ball.y;
    int16_t bound_x = ball.radius / 2;
    int16_t bound_y = ball.radius / 2;

    int16_t target_x = player.x + (player.width / 2);
    int16_t target_y = player.y;
    int16_t target_bound_x = player.width / 2;
    int16_t target_bound_y = player.height / 2;

    if (__Arkanoid_Physic_HasCollision(
        x,
        y,
        bound_x,
        bound_y,
        target_x,
        target_y,
        target_bound_x,
        target_bound_y)) {
        ball.direction_y *= -1;
    } else {
        for (int i = 0; i < scene.blocks_size; i++) {
            if (!blocks[i].is_alive) {
                continue;
            }

            target_bound_x = blocks[i].width / 2;
            target_bound_y = blocks[i].height / 2;
            target_x = blocks[i].x + target_bound_x;
            target_y = blocks[i].y + target_bound_y;
            if (__Arkanoid_Physic_HasCollision(
                x,
                y,
                bound_x,
                bound_y,
                target_x,
                target_y,
                target_bound_x,
                target_bound_y)) {
                blocks[i].is_destroying = 1;
                blocks[i].is_alive = 0;
                ball.direction_y *= -1;
                player.score += blocks[i].score;
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

    scene.is_clean_ball = 1;

    ball.previous_x = ball.x;
    ball.previous_y = ball.y;

    if (ball.x >= scene.width || ball.x <= 0) {
        ball.direction_x *= -1;
    } else if (ball.y <= 0) {
        ball.direction_y *= -1;
    } else if (ball.y >= scene.height) {
        scene.status = ARKANOID_GAME_STATUS_LOSE;
        //ball.direction_y *= -1;
    }

    ball.x += ball.velocity_x * ball.direction_x;
    ball.y += ball.velocity_y * ball.direction_y;

}

void Arkanoid_WorldUpdate()
{

    if (scene.status != ARKANOID_GAME_STATUS_PLAYING) {
        return;
    }

    __Arkanoid_Physic_MoveBall();
    __Arkanoid_Physic_ColissionDetection();

    uint8_t has_alive_blocks = 0;
    for (int i = 0; i < scene.blocks_size; i++) {
        if (blocks[i].is_alive) {
            has_alive_blocks = 1;
            break;
        }
    }

    if (!has_alive_blocks) {
        scene.status = ARKANOID_GAME_STATUS_WIN;
    }
}

__weak void Arkanoid_HandleRusult()
{
    if (scene.status == ARKANOID_GAME_STATUS_WIN) {
        scene.draw_engine->cursor((scene.width / 2) - 40, scene.height / 2);
        scene.draw_engine->background(scene.draw_engine->backgroundColor);
        scene.draw_engine->printf("WIN!");
    } else {
        scene.draw_engine->background(scene.draw_engine->backgroundColor);
        scene.draw_engine->cursor((scene.width / 2) - 10, scene.height / 2);
        scene.draw_engine->printf("Game Over!");
    }
}
