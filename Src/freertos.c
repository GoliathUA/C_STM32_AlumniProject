/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * Copyright (c) 2018 STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include <3Dbox.h>
#include <cmsis_os.h>
#include <eMPL/inv_mpu.h>
#include <i2c.h>
#include <lcd.h>
#include <mpu_drv.h>
#include <MadgwickAHRS.h>
#include <ringbuffer_dma.h>
#include <stddef.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_uart.h>
#include <sys/_stdint.h>
#include <usart.h>

#include "app.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId displayTaskHandle;
osThreadId sensorsTaskHandle;
osThreadId transmitTaskHandle;
osMutexId sensorsMutexHandle;
osMutexId appMutexHandle;

/* USER CODE BEGIN Variables */

osMailQId mailYPRHandle;
osMailQId mailCMDHandle;

/*
 char cmd[16];
 uint8_t cmd_index;
 char cmd_params[128];
 uint8_t is_cmd_params;
 uint8_t cmd_params_index;
 */

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDisplayTask(void const * argument);
void StartSensorsTask(void const * argument);
void StartCommunicationTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

    App_Init(&huart4, &hadc1, &hi2c1, &appMutexHandle, &sensorsMutexHandle);

    /* USER CODE END Init */

    /* Create the mutex(es) */
    /* definition and creation of sensorsMutex */
    osMutexDef(sensorsMutex);
    sensorsMutexHandle = osMutexCreate(osMutex(sensorsMutex));

    /* definition and creation of appMutex */
    osMutexDef(appMutex);
    appMutexHandle = osMutexCreate(osMutex(appMutex));

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* Create the thread(s) */
    /* definition and creation of displayTask */
    osThreadDef(displayTask, StartDisplayTask, osPriorityLow, 0, 512);
    displayTaskHandle = osThreadCreate(osThread(displayTask), NULL);

    /* definition and creation of sensorsTask */
    osThreadDef(sensorsTask, StartSensorsTask, osPriorityNormal, 0, 512);
    sensorsTaskHandle = osThreadCreate(osThread(sensorsTask), NULL);

    /* definition and creation of transmitTask */
    osThreadDef(transmitTask, StartCommunicationTask, osPriorityHigh, 0, 512);
    transmitTaskHandle = osThreadCreate(osThread(transmitTask), NULL);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    osMailQDef(mailState, 16, APP_StateTypeDef);


    app.statesQueueHandle = osMailCreate(osMailQ(mailState), NULL);

    App_State("menu");
    /* USER CODE END RTOS_QUEUES */
}

/* StartDisplayTask function */
void StartDisplayTask(void const * argument)
{

    /* USER CODE BEGIN StartDisplayTask */
    /* Infinite loop */
    osEvent evt;
    osStatus status;
    APP_StateTypeDef* state;

    for (;;) {

        evt = osMailGet(app.statesQueueHandle, 10);
        if (evt.status == osEventMail) {
            /* Obtain data from mail */
            state = (APP_StateTypeDef *) evt.value.p;
            /* Free the cell in mail queue */
            osMailFree(mailYPRHandle, state);

            status = osMutexWait(appMutexHandle, osWaitForever);
            if (status == osOK) {
                app.state = state;
                osMutexRelease(appMutexHandle);
            }

        }

        if (app.state != NULL && app.state->is_update) {
            status = osMutexWait(appMutexHandle, osWaitForever);
            if (status == osOK) {
                app.state->displayHandle(app.state->name);
                app.state->is_update = 0;
                osMutexRelease(appMutexHandle);
            }
        }

        osDelay(40);
    }
    /* USER CODE END StartDisplayTask */
}

/* StartSensorsTask function */
void StartSensorsTask(void const * argument)
{
    /* USER CODE BEGIN StartSensorsTask */
    osStatus status;
    /* Infinite loop */

    for (;;) {

        status = osMutexWait(sensorsMutexHandle, osWaitForever);
        if (status == osOK) {

            if (app.state != NULL && app.state->dataHandle != NULL) {
                app.state->dataHandle(app.state->name);
            }

            osMutexRelease(sensorsMutexHandle);
        }

        osDelay(20);
    }
    /* USER CODE END StartSensorsTask */
}

/* StartCommunicationTask function */
void StartCommunicationTask(void const * argument)
{
    /* USER CODE BEGIN StartCommunicationTask */
    /* Infinite loop */

    /* Infinite loop */
    for (;;) {
        App_OnReadRemoteCommand();
        osDelay(60);
    }
    /* USER CODE END StartCommunicationTask */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
