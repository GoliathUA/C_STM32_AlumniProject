#include "BMP280.h"
#include "app_util.h"

#define METEO_HISTORY_LENGTH 100
#define METEO_HISTORY_INTERVAL 600000 // 10 minutes

typedef enum
{
    METEO_HISTORY_ONE_HOUR,
    METEO_HISTORY_TWO_HOUR,
    METEO_HISTORY_THREE_HOUR,
    METEO_HISTORY_LAST
} METEO_HISTORY;

struct
{
    APP_RenderingEngineTypeDef *display;
    bmp280_t bmp280;
    uint8_t isBmpWorking;
    uint16_t QNH;
    double temp, press, alt;
    char* forecast;

    double _history[METEO_HISTORY_LENGTH];
    uint16_t _historyIndex;
    uint32_t _historyLastTime;

} METEO_DataStruct;

void METEO_Init(
    I2C_HandleTypeDef *i2c,
    APP_RenderingEngineTypeDef *display,
    uint16_t QNH);
void METEO_Draw();

void METEO_UpdateData();
char* METEO_GetForecast();
char* METEO_GetEasyForecast();

void __METEO_AddForecastHistory(double press);
double __METEO_GetForecastHistory(uint16_t interval);

