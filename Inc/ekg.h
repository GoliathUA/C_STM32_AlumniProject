#include "app_util.h"

typedef struct
{
    APP_RenderingEngineTypeDef *display;
    ADC_HandleTypeDef* adc;
    uint32_t channel;

    uint32_t x, currentY, prevY, ratio, currentValue;
} EKG_GraphTypeDef;

volatile EKG_GraphTypeDef ekg;

void EKG_Init(
    APP_RenderingEngineTypeDef *display,
    ADC_HandleTypeDef* adc,
    uint32_t channel);

void EKG_Draw();
