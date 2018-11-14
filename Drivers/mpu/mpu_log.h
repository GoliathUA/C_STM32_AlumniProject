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
/*
 * MPU_log.h
 *
 * Provides logging functions interface for eMPL
 */

#ifndef __MPU_LOG_H
#define __MPU_LOG_H

#include "mpu.h"
#if defined(LOG_UART)
void mpu_log_i(const char *fmt, ...);
#define mpu_log_e	mpu_log_i
#elif defined(LOG_LCD)
#include "lcd.h"
#define mpu_log_i	LCD_Printf
#define mpu_log_e	LCD_Printf
#else
#define mpu_log_i(...)	do{} while(0);
#define mpu_log_e(...)	do{} while(0);
#endif

#endif /* __MPU_LOG_H */
