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
 * mpu_log.c
 *
 * Provides logging functions interface for eMPL
 */
#include "mpu_log.h"

#if defined(LOG_UART)
#include <stdio.h>
#include <stdarg.h>
#include "stm32f4xx.h"
#include "usart.h"

void mpu_log_i(const char *fmt, ...) {
	static char buf[256];
	int len = 0;
	va_list lst;

	va_start(lst, fmt);
	len = vsprintf(buf, fmt, lst);
	va_end(lst);

	HAL_UART_Transmit(&huart2, (uint8_t *) buf, len, 1000);
}
#endif
