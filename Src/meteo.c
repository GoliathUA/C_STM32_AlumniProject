#include "meteo.h"

void METEO_Init(
    I2C_HandleTypeDef *i2c,
    APP_RenderingEngineTypeDef *display,
    uint16_t QNH)
{
    METEO_DataStruct.QNH = QNH;
    METEO_DataStruct.display = display;

    int8_t com_rslt;

    METEO_DataStruct.bmp280.i2c_handle = i2c;
    METEO_DataStruct.bmp280.dev_addr = BMP280_I2C_ADDRESS1;
    com_rslt = BMP280_init(&METEO_DataStruct.bmp280);
    com_rslt += BMP280_set_power_mode(BMP280_NORMAL_MODE);
    com_rslt += BMP280_set_work_mode(BMP280_STANDARD_RESOLUTION_MODE);
    com_rslt += BMP280_set_standby_durn(BMP280_STANDBY_TIME_1_MS);

    METEO_DataStruct.isBmpWorking = (com_rslt == SUCCESS);
}

void METEO_Draw()
{
    APP_RenderingEngineTypeDef* display = METEO_DataStruct.display;

    display->cursor(0, 0);

    display->printf("METEO Service:\n\n");

    display->printf("\tTemp : %6.2f C\n", METEO_DataStruct.temp);
    display->printf("\tPress: %6.0f Pa\n", METEO_DataStruct.press);
    display->printf("\tAlt  : %3.0f m\n\n", METEO_DataStruct.alt);

    display->printf("\tForecast  : %s", METEO_DataStruct.forecast);



}

void METEO_UpdateData()
{
    BMP280_read_temperature_double(&METEO_DataStruct.temp);
    BMP280_read_pressure_double(&METEO_DataStruct.press);
    /* Calculate current altitude, based on current QNH pressure */
    METEO_DataStruct.alt = BMP280_calculate_altitude(
        METEO_DataStruct.QNH * 100);

    METEO_DataStruct.forecast = METEO_GetForecast();
}

char* METEO_GetForecast()
{
    // XXX: 1 hPa = 100 Pa
    double press = METEO_DataStruct.press / 100;

    if (press < 1009) {
        return "rainy";
    }

    if (press < 1013) {
        return "cloudy";
    }

    if (press < 1014.5) {
        return "unsettled";
    }

    if (press < 1016) {
        return "sunny";
    }

    if (press >= 1016) {
        return "sun";
    }

    return "N/A";
}
