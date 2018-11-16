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
    ekg.x = 0;
    ekg.currentValue = 0;
    ekg.ratio = 18;

    ekg.prevY = APP_ADC_GetValue(ekg.adc, ekg.channel);
}

void EKG_Draw()
{
    LCD_SetRotation(1);

    LCD_FillRect(ekg.x, 10, 1, TFTWIDTH - 10, BLUE);
    ekg.currentValue = APP_ADC_GetValue(ekg.adc, ekg.channel);
    ekg.currentY = TFTWIDTH - ekg.currentValue / ekg.ratio;
    LCD_SetCursor(0, 0);
    LCD_Printf("VALUE: %4d", ekg.currentValue);
    LCD_DrawLine(ekg.x - 1, ekg.prevY, ekg.x, ekg.currentY, WHITE);
    ekg.x = (ekg.x + 1) % TFTHEIGHT;
    ekg.prevY = ekg.currentY;

    LCD_SetRotation(0);
    HAL_Delay(50);

}
