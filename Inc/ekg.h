#include "app_util.h"

typedef struct
{
    uint32_t x, y, value;
} EKG_GraphDataTypeDef;

typedef struct
{
    APP_RenderingEngineTypeDef *display;
    ADC_HandleTypeDef* adc;
    uint32_t channel, ratio, global_x;

    EKG_GraphDataTypeDef prev_data;
} EKG_GraphTypeDef;

volatile EKG_GraphTypeDef ekg;

void EKG_Init(
    APP_RenderingEngineTypeDef *display,
    ADC_HandleTypeDef* adc,
    uint32_t channel);

void EKG_Draw();
void EKG_Draw_Interval(EKG_GraphDataTypeDef prev, EKG_GraphDataTypeDef current);
void EKG_Draw_Point(EKG_GraphDataTypeDef current);

EKG_GraphDataTypeDef EKG_Graph_Get_Data();

