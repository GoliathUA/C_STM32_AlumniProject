#include "BMP280.h"
#include "app_util.h"

struct
{
    APP_RenderingEngineTypeDef *display;
    bmp280_t bmp280;
    uint8_t isBmpWorking;
    uint16_t QNH;
    double temp, press, alt;
    char* forecast;

} METEO_DataStruct;

void METEO_Init(
    I2C_HandleTypeDef *i2c,
    APP_RenderingEngineTypeDef *display,
    uint16_t QNH);
void METEO_Draw();


void METEO_UpdateData();
char* METEO_GetForecast();
