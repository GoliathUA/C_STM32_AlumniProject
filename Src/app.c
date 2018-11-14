#include "app.h"

void App_Init(
    UART_HandleTypeDef *uart,
    ADC_HandleTypeDef *adc,
    I2C_HandleTypeDef *i2c,
    osMutexId *sensorsMutexHandle,
    osMutexId *appMutexHandle)
{
    app.uart = uart;
    app.adc = adc;
    app.i2c = i2c;
    app.sensorsMutexHandle = sensorsMutexHandle;
    app.appMutexHandle = appMutexHandle;

    LCD_Init();

#if APP_DEBUG_MODE
    LCD_Printf("Initing...\n");
#endif

    __App_Init_RemoteCommand();
    __App_Init_BMP();
    __App_Init_States();
    __App_Init_Harvesters();

    /* Start ADC using DMA with TIM8 Trigger */
    HAL_ADC_Start_DMA(app.adc, (uint32_t *) app_buffers.adc, 2);

    /*
     HAL_TIM_Base_Start(&htim8);

     RingBuffer_DMA_Init(&rx_buf, app.uart->hdmarx, rx, APP_READ_BUFER_SIZE);
     HAL_UART_Receive_DMA(app.uart, rx, APP_READ_BUFER_SIZE);







     APP_SetCallbackMapItem(app.listeners, "adc", App_Handle_Command_ADC);
     APP_SetCallbackMapItem(app.listeners, "led", App_Handle_Command_LED);
     APP_SetCallbackMapItem(app.listeners, "clear", App_Handle_Command_Clean);
     APP_SetCallbackMapItem(app.listeners, "bmp", App_Handle_Command_BMP);

     LCD_Printf("DONE\n");
     */
}

void __App_Init_RemoteCommand()
{
    remote_command.cmd_index = remote_command.is_cmd_params =
            remote_command.cmd_params_index = 0;

    app.listeners = APP_CreateCallbackMap(3);

    if (app.listeners == NULL) {
#if APP_DEBUG_MODE
        LCD_Printf("\t+ Bluetooth Disabled\n");
#endif
        return;
    }

    APP_SetCallbackMapItem(app.listeners, "ping", App_Handle_Command_Ping);
    APP_SetCallbackMapItem(app.listeners, "menu", App_Handle_Command_Menu);

    RingBuffer_DMA_Init(
        &remote_command.rx_buf,
        app.uart->hdmarx,
        remote_command.rx,
        APP_READ_BUFER_SIZE);
    HAL_UART_Receive_DMA(app.uart, remote_command.rx, APP_READ_BUFER_SIZE);

#if APP_DEBUG_MODE
    LCD_Printf("\t+ Bluetooth Available\n");
#endif
}

void __App_Init_States(void)
{
    app.states = APP_CreateCallbackMap(3);

    APP_SetCallbackMapItem(app.states, "menu", App_Handle_State_Menu);
    APP_SetCallbackMapItem(app.states, "meteo", App_Handle_State_Meteo);

}

void __App_Init_Harvesters(void)
{
    app.harvesters = APP_CreateCallbackMap(1);

    APP_SetCallbackMapItem(app.harvesters, "meteo", App_Handle_Harvester_Meteo);
}

void __App_Init_BMP(void)
{
    app.is_bmp_working = 0;

    int8_t com_rslt;

    app.bmp280.i2c_handle = app.i2c;
    app.bmp280.dev_addr = BMP280_I2C_ADDRESS1;
    com_rslt = BMP280_init(&app.bmp280);
    com_rslt += BMP280_set_power_mode(BMP280_NORMAL_MODE);
    com_rslt += BMP280_set_work_mode(BMP280_STANDARD_RESOLUTION_MODE);
    com_rslt += BMP280_set_standby_durn(BMP280_STANDBY_TIME_1_MS);
    if (com_rslt != SUCCESS) {
#if APP_DEBUG_MODE
        LCD_Printf("\t- BMP280 Disabled\n");
#endif
        return;
    }

    meteo_data.QNH = 1013;
    app.is_bmp_working = 1;

#if APP_DEBUG_MODE
    LCD_Printf("\t+ BMP280 Available\n");
#endif
}

void App_OnReadRemoteCommand()
{
    uint32_t rx_count = RingBuffer_DMA_Count(&remote_command.rx_buf);
    while (rx_count--) {
        uint8_t b = RingBuffer_DMA_GetByte(&remote_command.rx_buf);
        if (b == '\r' || b == '\n') {
            remote_command.cmd[remote_command.cmd_index] = 0;
            remote_command.cmd_params[remote_command.cmd_params_index] = 0;
            remote_command.cmd_index = remote_command.cmd_params_index =
                    remote_command.is_cmd_params = 0;

            App_Exec_Command(remote_command.cmd, remote_command.cmd_params);
            break;
        } else if (remote_command.cmd_index > 0 && b == ' '
                && !remote_command.is_cmd_params) {
            remote_command.is_cmd_params = 1;
        } else if (remote_command.is_cmd_params) {
            remote_command.cmd_params[remote_command.cmd_params_index++] = b;
        } else {
            remote_command.cmd[remote_command.cmd_index++] = b;
        }
    }
}

void App_Exec_Command(char *command, char *params)
{
    APP_CallbackTypeDef callback = APP_GetCallbackMapItem(
        app.listeners,
        command);

    if (callback == NULL) {
        LCD_Printf("Undefined command: %s\n", command);
        return;
    }

#if APP_DEBUG_MODE
    LCD_Printf("[DEBUG] CMD: %s [%s]\n", command, params);
#endif
    callback(params);
#if APP_DEBUG_MODE
    LCD_Printf("\n\n[DEBUG] CMD: %s - DONE\n", command);
#endif
}

void App_State(char *name)
{
    APP_CallbackTypeDef callback = APP_GetCallbackMapItem(app.states, name);

    if (callback == NULL) {
        LCD_Printf("Undefined State: %s\n", name);
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
        state->is_update = 1;

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
    LCD_Printf("Sending: %s\n", data);
#endif

    HAL_StatusTypeDef status = HAL_UART_Transmit(
        app.uart,
        data,
        strlen((char *) data),
        APP_DEFAULT_TIMEOUT);

    if (status != HAL_OK) {
        LCD_Printf("Send Error: %s\n", data);
    }
}

void App_UpdateSatet(void)
{
    app.state->is_update = 1;
}

/////////////////////////
//  State Handlers
/////////////////////////

void App_Handle_State_Menu(char *state)
{
    LCD_FillScreen(BLACK);
    LCD_SetCursor(0, 0);

    char* menu_items[] = { "Meteo Service", "3D Box", "EKG", "Arkanoid" };

    LCD_Printf("Menu:\n\n");
    for (int i = 0; i < sizeof(menu_items) / sizeof(char*); i++) {
        LCD_Printf("[%d] %s\n", (i + 1), menu_items[i]);
    }

    LCD_Printf("\n\nPlease send command: menu N");
}

void App_Handle_State_Meteo(char *state)
{
    LCD_FillScreen(BLACK);
    LCD_SetCursor(0, 0);

    LCD_Printf("METEO Service:\n\n");

    LCD_Printf("\tTemp : %6.2f C\n", meteo_data.temp);
    LCD_Printf("\tPress: %6.0f Pa\n", meteo_data.press);
    LCD_Printf("\tAlt  : %3.0f m", meteo_data.alt);

    LCD_Printf("\n\n Back to menu send command: menu");
}

/////////////////////////
//  Command Handlers
/////////////////////////

void App_Handle_Command_Ping(char *params)
{
    uint8_t data[] = "pong\r\n";
    App_Send_Data(data);
}

void App_Handle_Command_Menu(char *params)
{
    uint8_t index = atoi(params);

    if (!index) {
        App_State("menu");
        return;
    }

    App_State("meteo");
}

/////////////////////////
//  Command Harvesters
/////////////////////////

void App_Handle_Harvester_Meteo(char *state)
{

    double temp, press, alt;
    BMP280_read_temperature_double(&temp);
    BMP280_read_pressure_double(&press);
    /* Calculate current altitude, based on current QNH pressure */
    alt = BMP280_calculate_altitude(meteo_data.QNH * 100);

    if (meteo_data.temp != temp || meteo_data.press != press
            || meteo_data.alt != alt) {
        App_UpdateSatet();
    }

    meteo_data.temp = temp;
    meteo_data.press = press;
    meteo_data.alt = alt;

}

