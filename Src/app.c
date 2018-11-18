#include "app.h"

void App_Init(
    UART_HandleTypeDef *uart,
    ADC_HandleTypeDef *adc,
    I2C_HandleTypeDef *i2c)
{
    app.uart = uart;
    app.adc = adc;
    app.i2c = i2c;

    __App_Init_Rendering();

#if APP_DEBUG_MODE
    app.display.printf("Initing...\n");
#endif

    __App_Init_RemoteCommand();
    __App_Init_Meteo();
    __App_Init_States();
    __App_Init_Harvesters();
    __App_Init_MPU();
    __App_Init_Arkanoid();
    __App_Init_EKG();
}

void __App_Init_Rendering(void)
{
#if defined(__LCD_H)
    app.display.rect = LCD_FillRect;
    app.display.color = LCD_Color565;
    app.display.circle = LCD_FillCircle;
    app.display.printf = LCD_Printf;
    app.display.cursor = LCD_SetCursor;
    app.display.background = LCD_FillScreen;
    app.display.backgroundColor = BLACK;
    app.display.screenWidth = TFTWIDTH;
    app.display.screenHeight = TFTHEIGHT;
    app.display.line = LCD_DrawLine;

    LCD_Init();

#elif defined(__OLED_H)
    app.display.rect = OLED_FillRect;
    app.display.color = OLED_Color565;
    app.display.circle = OLED_FillCircle;
    app.display.printf = OLED_Printf;
    app.display.cursor = OLED_SetCursor;
    app.display.background = OLED_FillScreen;
    app.display.backgroundColor = OLED_Color_t.Black;
    app.display.screenWidth = OLEDWIDTH;
    app.display.screenHeight = OLEDHEIGHT;
    app.display.line = OLED_DrawLine;

    if (OLED_Init(&app.i2c) != HAL_OK) {
        Error_Handler();
    }

#else
    error Undefined display lib
#endif

}

void __App_Init_Arkanoid(void)
{
    Arkanoid_Init(&app.display);
}

void __App_Init_EKG(void)
{
    EKG_Init(&app.display, app.adc, ADC_CHANNEL_14);
}

void __App_Init_RemoteCommand(void)
{
    remote_command.cmdIndex = remote_command.isCmdParams =
            remote_command.cmdParamsIndex = 0;

    app.listeners = APP_CreateCallbackMap(3);

    if (app.listeners == NULL) {
#if APP_DEBUG_MODE
        app.display.printf("\t+ Bluetooth Disabled\n");
#endif
        return;
    }

    APP_SetCallbackMapItem(app.listeners, "ping", App_Handle_Command_Ping);
    APP_SetCallbackMapItem(app.listeners, "menu", App_Handle_Command_Menu);
    APP_SetCallbackMapItem(app.listeners, "meteo", App_Handle_Command_Meteo);

    RingBuffer_DMA_Init(
        &remote_command.rxBuffer,
        app.uart->hdmarx,
        remote_command.rx,
        APP_READ_BUFER_SIZE);
    HAL_UART_Receive_DMA(app.uart, remote_command.rx, APP_READ_BUFER_SIZE);

#if APP_DEBUG_MODE
    app.display.printf("\t+ Bluetooth Available\n");
#endif
}

void __App_Init_States(void)
{
    app.states = APP_CreateCallbackMap(5);

    APP_SetCallbackMapItem(app.states, "menu", App_Handle_State_Menu);
    APP_SetCallbackMapItem(app.states, "meteo", App_Handle_State_Meteo);
    APP_SetCallbackMapItem(app.states, "motion", App_Handle_State_Motion);
    APP_SetCallbackMapItem(app.states, "arkanoid", App_Handle_State_Arkanoid);
    APP_SetCallbackMapItem(app.states, "ekg", App_Handle_State_EKG);
}

void __App_Init_Harvesters(void)
{
    app.harvesters = APP_CreateCallbackMap(4);

    APP_SetCallbackMapItem(app.harvesters, "meteo", App_Handle_Harvester_Meteo);
    APP_SetCallbackMapItem(
        app.harvesters,
        "motion",
        App_Handle_Harvester_Motion);
    APP_SetCallbackMapItem(
        app.harvesters,
        "arkanoid",
        App_Handle_Harvester_Arkanoid);
    APP_SetCallbackMapItem(app.harvesters, "ekg", App_Handle_Harvester_EKG);
}

void __App_Init_Meteo(void)
{
    METEO_Init(app.i2c, &app.display, APP_QNH);

    if (!METEO_DataStruct.isBmpWorking) {
#if APP_DEBUG_MODE
        app.display.printf("\t- BMP280 Disabled\n");
#endif
        return;
    }

#if APP_DEBUG_MODE
    app.display.printf("\t+ BMP280 Available\n");
#endif
}

void __App_Init_MPU(void)
{
    /* Start MPU9250 and change settings */
    mpu_i2c_init(app.i2c);
    if (mpu_init(NULL)) {
#if APP_DEBUG_MODE
        app.display.printf("\tMPU9250 Disabled\n");
#endif
        return;
    }

    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
    mpu_set_sample_rate(500);

    motion_data.accScale = 0;
    motion_data.gyroScale = 0.f;

    mpu_get_accel_sens(&motion_data.accScale);
    mpu_get_gyro_sens(&motion_data.gyroScale);

    Madgwick_init();
    motion_data.lastTime = HAL_GetTick();
    motion_data.index = 0;
    motion_data.magScale = 32760.0f / 4912.0f;

    motion_data.boxWidth = 120;
    motion_data.boxDepth = 80;
    motion_data.boxHeight = 10;

    Object3d_InitBox(
        &motion_data.box,
        motion_data.boxWidth,
        motion_data.boxDepth,
        motion_data.boxHeight);

#if APP_DEBUG_MODE
    app.display.printf("\t+ MPU9250 Available\n");
#endif
}

void App_OnReadRemoteCommand()
{
    uint32_t rx_count = RingBuffer_DMA_Count(&remote_command.rxBuffer);
    while (rx_count--) {
        uint8_t b = RingBuffer_DMA_GetByte(&remote_command.rxBuffer);
        if (b == '\r' || b == '\n') {
            remote_command.cmd[remote_command.cmdIndex] = 0;
            remote_command.cmdParams[remote_command.cmdParamsIndex] = 0;
            remote_command.cmdIndex = remote_command.cmdParamsIndex =
                    remote_command.isCmdParams = 0;

            App_Exec_Command(remote_command.cmd, remote_command.cmdParams);
            break;
        } else if (remote_command.cmdIndex > 0 && b == ' '
                && !remote_command.isCmdParams) {
            remote_command.isCmdParams = 1;
        } else if (remote_command.isCmdParams) {
            remote_command.cmdParams[remote_command.cmdParamsIndex++] = b;
        } else {
            remote_command.cmd[remote_command.cmdIndex++] = b;
        }
    }
}

void App_Exec_Command(char *command, char *params)
{
    APP_CallbackTypeDef callback = APP_GetCallbackMapItem(
        app.listeners,
        command);

    if (callback == NULL) {
        app.display.printf("Undefined command: %s\n", command);
        return;
    }

#if APP_DEBUG_MODE
    app.display.printf("[DEBUG] CMD: %s [%s]\n", command, params);
#endif
    callback(params);
#if APP_DEBUG_MODE
    app.display.printf("\n\n[DEBUG] CMD: %s - DONE\n", command);
#endif
}

void App_State(char *name)
{
    APP_CallbackTypeDef callback = APP_GetCallbackMapItem(app.states, name);

    if (callback == NULL) {
        app.display.printf("Undefined State: %s\n", name);
        return;
    }

    APP_CallbackTypeDef dataHandle = APP_GetCallbackMapItem(
        app.harvesters,
        name);

    APP_StateTypeDef* state = (APP_StateTypeDef *) osMailAlloc(
        app.statesQueueHandle,
        5);

    if (state != NULL) {
        state->name = name;
        state->displayHandle = callback;
        state->isUpdate = 1;

        if (dataHandle != NULL) {
            state->dataHandle = dataHandle;
        } else {
            state->dataHandle = NULL;
        }

        osMailPut(app.statesQueueHandle, state);
    }
}

void App_Send_Data(uint8_t *data)
{

#if APP_DEBUG_MODE
    app.display.printf("Sending: %s\n", data);
#endif

    HAL_StatusTypeDef status = HAL_UART_Transmit(
        app.uart,
        data,
        strlen((char *) data),
        APP_DEFAULT_TIMEOUT);

    if (status != HAL_OK) {
        app.display.printf("Send Error: %s\n", data);
    }
}

void App_UpdateSatet(void)
{
    app.state->isUpdate = 1;
}

/////////////////////////
//  State Handlers
/////////////////////////

void App_Handle_State_Menu(char *state)
{
    LCD_FillScreen(BLACK);
    LCD_SetCursor(0, 0);

    char* menu_items[] = { "Meteo Service", "Motion", "EKG", "Arkanoid" };

    app.display.printf("Menu:\n\n");
    for (int i = 0; i < sizeof(menu_items) / sizeof(char*); i++) {
        app.display.printf("[%d] %s\n", (i + 1), menu_items[i]);
    }

    app.display.printf("\n\nPlease send command: menu N");
}

void App_Handle_State_Meteo(char *state)
{
    METEO_Draw();

    app.display.printf("\n\n Back to menu send command: menu");
}

void App_Handle_State_Motion(char *state)
{
    osEvent evt = osMailGet(motion_data.queueHandle, osWaitForever);

    if (evt.status == osEventMail) {
        /* Obtain data from mail */
        APP_MotionTypeDef* data = (APP_MotionTypeDef *) evt.value.p;
        /* Redraw 3Dbox*/
        Object3d_CleanObject(&motion_data.box);
        Object3d_SetRotation(
            &motion_data.box,
            data->pitch,
            data->roll,
            data->yaw);

        Object3d_DrawObject(&motion_data.box);

        osMailFree(motion_data.queueHandle, data);

        //Object3d_SetEdgesColor(&box, WHITE);
    }
}

void App_Handle_State_EKG(char *state)
{
    EKG_Draw();
}

/////////////////////////
//  Command Handlers
/////////////////////////

void App_Handle_Command_Ping(char *params)
{
    uint8_t data[] = "pong\r\n";
    App_Send_Data(data);
}

void App_Handle_Command_Meteo(char *params)
{
    if (strcmp(app.state->name, "meteo") != 0) {
        uint8_t data[] = "Disabled command in current window.\r\n";
        App_Send_Data(data);
        return;
    }

    uint8_t response[100];
    sprintf(
        (char *) response,
        "temp=%f; press=%f; alt=%f; forecast=%s\r\n",
        METEO_DataStruct.temp,
        METEO_DataStruct.press,
        METEO_DataStruct.alt,
        METEO_DataStruct.forecast);

    App_Send_Data(response);
}

void App_Handle_Command_Menu(char *params)
{
    uint8_t index = atoi(params);

    char* menu_items[] = { "menu", "meteo", "motion", "ekg", "arkanoid" };

    if (index > (sizeof(menu_items) / sizeof(char*))) {
#if APP_DEBUG_MODE
        app.display.printf("Error unknown menu index");
#endif
        return;
    }

    App_State(menu_items[index]);
}

/////////////////////////
//  Command Harvesters
/////////////////////////

void App_Handle_State_Arkanoid(char *state)
{
    Arkanoid_Draw();
}

void App_Handle_Harvester_Meteo(char *state)
{
    double temp = METEO_DataStruct.temp;
    double press = METEO_DataStruct.press;
    double alt = METEO_DataStruct.alt;

    METEO_UpdateData();

    if (METEO_DataStruct.temp != temp || METEO_DataStruct.press != press
            || METEO_DataStruct.alt != alt) {
        App_UpdateSatet();
    }
}

void App_Handle_Harvester_Motion(char *state)
{
    mpu_get_int_status(&motion_data.intStatus);

    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;

    uint32_t currentTime;

    if (motion_data.intStatus & MPU_INT_STATUS_DATA_READY) {
        /* Get all raw measurements */
        mpu_get_accel_reg(motion_data.acc, 0);
        mpu_get_gyro_reg(motion_data.gyro, 0);
        mpu_get_compass_reg(motion_data.mag, 0);
        /* Convert to real units */
        ax = (float) (motion_data.acc[0]) / motion_data.accScale;
        ay = (float) (motion_data.acc[1]) / motion_data.accScale;
        az = (float) (motion_data.acc[2]) / motion_data.accScale;
        gx = motion_data.gyro[0] / motion_data.gyroScale;
        gy = motion_data.gyro[1] / motion_data.gyroScale;
        gz = motion_data.gyro[2] / motion_data.gyroScale;
        mx = motion_data.mag[1] / motion_data.magScale;
        my = motion_data.mag[0] / motion_data.magScale;
        mz = -motion_data.mag[2] / motion_data.magScale;
        /* Do data processing by Madgwick filter */
        currentTime = HAL_GetTick();
        Madgwick_update(
            gx,
            gy,
            gz,
            ax,
            ay,
            az,
            mx,
            my,
            mz,
            (currentTime - motion_data.lastTime) / 1000.0);
        motion_data.lastTime = currentTime;
    }

    if (motion_data.index++ % 4 == 0) {
        /* Allocate new cell in mail queue */
        APP_MotionTypeDef* data = (APP_MotionTypeDef *) osMailAlloc(
            motion_data.queueHandle,
            5);
        if (data != NULL) {
            /* Fill in data */
            data->pitch = Madgwick_getPitchRadians();
            data->roll = Madgwick_getRollRadians();
            data->yaw = Madgwick_getYawRadians();
            /* Put the mail in the queue */
            osMailPut(motion_data.queueHandle, data);
            App_UpdateSatet();
        }
    }

}

void App_Handle_Harvester_Arkanoid(char *state)
{
    if (ARK_Scene.status == ARKANOID_GAME_STATUS_PLAYING) {
        Arkanoid_WorldUpdate();
        App_UpdateSatet();
    }
}

void App_Handle_Harvester_EKG(char *state)
{
    App_UpdateSatet();
}

///////////////////
// Override Functions
///////////////////

void Arkanoid_HandleRusult()
{
    if (ARK_Scene.status == ARKANOID_GAME_STATUS_WIN) {
        LCD_SetCursor((ARK_Scene.width / 2) - 40, ARK_Scene.height / 2);
        LCD_FillScreen(BLACK);
        LCD_SetTextColor(WHITE, BLACK);
        LCD_SetTextSize(1);
        app.display.printf("WIN!");
    } else {
        LCD_FillScreen(BLACK);
        LCD_SetTextColor(WHITE, BLACK);
        LCD_SetCursor((ARK_Scene.width / 2) - 10, ARK_Scene.height / 2);
        app.display.printf("Game Over!");
    }

    osDelay(1500);

    App_State("menu");

    Arkanoid_Init(&app.display);
}
