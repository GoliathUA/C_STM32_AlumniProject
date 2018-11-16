#include <ringbuffer_dma.h>
#include <lcd.h>
#include <i2c.h>
#include <mpu_drv.h>
#include <eMPL/inv_mpu.h>
#include <MadgwickAHRS.h>
#include <3Dbox.h>

#include "tim.h"
#include "cmsis_os.h"
#include "usart.h"
#include "adc.h"
#include "BMP280.h"
#include "arkanoid.h"
#include "ekg.h"

#define APP_DEBUG_MODE 1
#define APP_READ_BUFER_SIZE 256
#define APP_DEFAULT_TIMEOUT 100

typedef struct
{
    uint8_t is_update;
    char *name;
    APP_CallbackTypeDef displayHandle;
    APP_CallbackTypeDef dataHandle;

} APP_StateTypeDef;

typedef struct
{
    float yaw, pitch, roll;
} APP_MotionTypeDef;

struct
{
    uint16_t QNH;
    double temp, press, alt;

} meteo_data;

struct
{
    uint16_t accScale;
    short intStatus;
    float gyroScale, magScale;
    uint32_t lastTime, index;

    int16_t acc[3], gyro[3], mag[3];

    uint16_t boxWidth, boxDepth, boxHeight;
    Object3d__HandleTypeDef box;

    osMailQId queueHandle;

} motion_data;

struct
{
    RingBuffer_DMA rx_buf;
    uint8_t rx[APP_READ_BUFER_SIZE];

    uint8_t is_cmd_params;

    uint8_t cmd_index;
    char cmd[128];

    uint8_t cmd_params_index;
    char cmd_params[128];
} remote_command;

struct
{
    uint32_t adc[2];

} app_buffers;

struct
{

    ADC_HandleTypeDef *adc;
    UART_HandleTypeDef *uart;
    I2C_HandleTypeDef *i2c;

    bmp280_t bmp280;

    uint8_t is_bmp_working;

    APP_CallbackMapTypeDef *states;
    APP_CallbackMapTypeDef *listeners;
    APP_CallbackMapTypeDef *harvesters;

    osMailQId statesQueueHandle;


    APP_StateTypeDef *state;

    APP_RenderingEngineTypeDef renderingEngine;

} app;

void App_Init(
    UART_HandleTypeDef * uart,
    ADC_HandleTypeDef *adc,
    I2C_HandleTypeDef *i2c);
void __App_Init_BMP(void);
void __App_Init_MPU(void);
void __App_Init_RemoteCommand(void);
void __App_Init_Harvesters(void);
void __App_Init_States(void);
void __App_Init_Arkanoid(void);
void __App_Init_Rendering(void);
void __App_Init_EKG(void);

void App_OnReadRemoteCommand(void);

void App_Exec_Command(char *command, char *params);
void App_UpdateSatet(void);
void App_Send_Data(uint8_t *data);
void App_State(char *state);

void App_Handle_Command_Ping(char *params);
void App_Handle_Command_Menu(char *params);

void App_Handle_State_Menu(char *state);
void App_Handle_State_Meteo(char *state);
void App_Handle_State_Motion(char *state);
void App_Handle_State_Arkanoid(char *state);
void App_Handle_State_EKG(char *state);

void App_Handle_Harvester_Meteo(char *state);
void App_Handle_Harvester_Motion(char *state);
void App_Handle_Harvester_Arkanoid(char *state);
void App_Handle_Harvester_EKG(char *state);
