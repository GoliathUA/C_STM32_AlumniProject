/**
*	TechMaker
*	https://techmaker.ua
*
*	STM32 MPU library for MPU6050/6500/9150/9250 using HAL library
*	based on Invensense eMPL library
*	03 May 2018 by Alexander Olenyev <sasha@techmaker.ua>
*
*	Changelog:
*		- v1.0 initial port to STM32
*/

#ifndef __MPU_H
#define __MPU_H

#include "stm32f4xx.h"
#include "eMPL/inv_mpu.h"
#include "eMPL/inv_mpu_dmp_motion_driver.h"
#include "mpu_drv.h"
#include "ml_math_func.h"

#define EMPL_TARGET_STM32F4

//#define MPU6050
//#define MPU6500
//#define MPU9150
#define MPU9250

//#define LOG_UART
//#define LOG_LCD

#if defined(LOG_UART) && !defined(HAL_UART_MODULE_ENABLED)
#error Please initialize UART module to output log
#endif

#ifndef EMPL_TARGET_STM32F4
#error Please define EMPL_TARGET_STM32F4 macro if you are using STM32 platform (any series)
#endif

#if !(defined(MPU6050) || defined(MPU6500) || defined(MPU9150) || defined(MPU9250))
#error Please select your MPUXXXX chip in MPU.h, lines 11-14
#endif

#endif /* __MPU_H */
