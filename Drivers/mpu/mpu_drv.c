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
 * mpu_drv.c
 *
 * Low-level I2C driver
 */
#include "mpu_drv.h"
#include "stm32f4xx.h"
// Hold pointer to initialized HAL I2C device
static I2C_HandleTypeDef * mpu_hi2c;

void mpu_i2c_init(I2C_HandleTypeDef *hi2c) {
	mpu_hi2c = hi2c;
}

inline int mpu_i2c_write(unsigned char slave_addr,
        unsigned char reg_addr,
        unsigned char length,
        unsigned char const *data) {
	return HAL_I2C_Mem_Write(mpu_hi2c, slave_addr << 1U, reg_addr, I2C_MEMADD_SIZE_8BIT, data, length, 100);
}

inline int mpu_i2c_read(unsigned char slave_addr,
        unsigned char reg_addr,
        unsigned char length,
        unsigned char *data) {
	return HAL_I2C_Mem_Read(mpu_hi2c, slave_addr << 1U, reg_addr, I2C_MEMADD_SIZE_8BIT, data, length, 100);
}
