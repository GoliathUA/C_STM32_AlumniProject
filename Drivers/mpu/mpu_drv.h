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
 * MPU_drv.h
 *
 * Low-level I2C driver
 */

#ifndef __MPU_DRV_H
#define __MPU_DRV_H

#include "stm32f4xx.h"

void mpu_i2c_init(I2C_HandleTypeDef *hi2c);
int mpu_i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const *data);
int mpu_i2c_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data);

#endif /* __MPU_DRV_H */
