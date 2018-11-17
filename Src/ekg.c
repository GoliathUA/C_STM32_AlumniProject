#include "ekg.h"
#include "lcd.h"

void EKG_Init(
    APP_RenderingEngineTypeDef *display,
    ADC_HandleTypeDef* adc,
    uint32_t channel)
{
    ekg.display = display;
    ekg.adc = adc;
    ekg.channel = channel;
    ekg.global_x = 0;
    ekg.ratio = 18;
}

EKG_GraphDataTypeDef EKG_Graph_Get_Data(uint32_t x)
{
    EKG_GraphDataTypeDef data;

    data.x = x;
    data.value = APP_ADC_GetValue(ekg.adc, ekg.channel);

    data.y = ekg.display->screenWidth - data.value / ekg.ratio;
    data.x = (data.x + 1) % ekg.display->screenHeight;

    return data;
}

void EKG_Draw_Interval(EKG_GraphDataTypeDef prev, EKG_GraphDataTypeDef current)
{
    ekg.display->cursor(0, 0);
    ekg.display->printf("VALUE: %4d", current.value);

    ekg.display->rect(
        ekg.global_x,
        10,
        1,
        ekg.display->screenWidth - 10,
        ekg.display->color(0, 0, 255));

    ekg.display->rect(
        ekg.global_x + 1,
        10,
        3,
        ekg.display->screenWidth - 10,
        ekg.display->backgroundColor);

    if (prev.x > current.x) {
        prev = current;
    }

    ekg.display->line(
        prev.x,
        prev.y,
        current.x,
        current.y,
        ekg.display->color(255, 255, 255));
}

void EKG_Draw_Point(EKG_GraphDataTypeDef current)
{
    ekg.global_x = current.x;

    if (!ekg.prev_data.value) {
        ekg.prev_data = current;
    }

    EKG_Draw_Interval(ekg.prev_data, current);

    ekg.prev_data = current;
}

void EKG_Draw()
{
    LCD_SetRotation(1);

    EKG_GraphDataTypeDef current = EKG_Graph_Get_Data(ekg.global_x);

    EKG_Draw_Point(current);

    LCD_SetRotation(0);
}
