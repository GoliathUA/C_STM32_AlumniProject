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
#include "meteo.h"

#define APP_DEBUG_MODE 0
#define APP_READ_BUFER_SIZE 256
#define APP_DEFAULT_TIMEOUT 100
#define APP_CMD_LENGTH 128
#define APP_QNH 1013

typedef struct
{
    uint8_t isUpdate;
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
    RingBuffer_DMA rxBuffer;
    uint8_t rx[APP_READ_BUFER_SIZE];

    uint8_t isCmdParams;

    uint8_t cmdIndex;
    char cmd[APP_CMD_LENGTH];

    uint8_t cmdParamsIndex;
    char cmdParams[APP_CMD_LENGTH];
} remote_command;

struct
{
    ADC_HandleTypeDef *adc;
    UART_HandleTypeDef *uart;
    I2C_HandleTypeDef *i2c;

    APP_CallbackMapTypeDef *states;
    APP_CallbackMapTypeDef *listeners;
    APP_CallbackMapTypeDef *harvesters;

    osMailQId statesQueueHandle;

    APP_StateTypeDef *state;

    APP_RenderingEngineTypeDef display;

} app;

void App_Init(
    UART_HandleTypeDef * uart,
    ADC_HandleTypeDef *adc,
    I2C_HandleTypeDef *i2c);
void __App_Init_Meteo(void);
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
void App_Handle_Command_Meteo(char *params);

void App_Handle_State_Menu(char *state);
void App_Handle_State_Meteo(char *state);
void App_Handle_State_Motion(char *state);
void App_Handle_State_Arkanoid(char *state);
void App_Handle_State_EKG(char *state);

void App_Handle_Harvester_Meteo(char *state);
void App_Handle_Harvester_Motion(char *state);
void App_Handle_Harvester_Arkanoid(char *state);
void App_Handle_Harvester_EKG(char *state);
