/**
*	TechMaker
*	https://techmaker.ua
*
*	STM32 driver for BMP280 chip with HAL library
*	based on Bosch Sensortec GmbH driver version 2.0.5
*	based on Bosch Sensortec GmbH datasheet rev. 1.14 5/15/2015 (BST-BMP280-DS001-11)
*	22 Jan 2018 by Alexander Olenyev <sasha@techmaker.ua>
*
*	Changelog:
*		- v1.0 initial release for STM32 HAL using I2C interface (SPI not yet supported)
*/

#ifndef __BMP280_DRV_H
#define __BMP280_DRV_H

#include "stm32f4xx.h"
extern bmp280_t *p_bmp280;

/**************************************************************/
/**\name	I2C BUS COMMUNICATION CUSTOM FUNCTIONS */
/**************************************************************/

#ifdef HAL_I2C_MODULE_ENABLED
/*!
 * @brief: The function is used as I2C bus write
 *
 *
 *
 *
 *	@param dev_addr : The device address of the sensor
 *	@param reg_addr : Address of the first register, where data is to be written
 *	@param reg_data : It is a value held in the array, which is written in the register
 *	@param cnt : The no of bytes of data to be written
 *
 *
 *	@return status of the I2C write
 *
 *
 */
static inline s8 BMP280_I2C_Write (u8 dev_addr,
		u8 reg_addr,
		u8 *reg_data,
		u8 cnt) {
	return HAL_I2C_Mem_Write(p_bmp280->i2c_handle, dev_addr << 1U, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, cnt, 100);
}
/*!
 * @brief: The function is used as I2C bus read
 *
 *
 *
 *
 *	@param dev_addr : The device address of the sensor
 *	@param reg_addr : Address of the first register, where data is going to be read
 *	@param reg_data : This is the data read from the sensor, which is held in an array
 *	@param cnt : The no of bytes of data to be read
 *
 *
 *	@return status of the I2C read
 *
 *
 */
static inline s8 BMP280_I2C_Read (u8 dev_addr,
		u8 reg_addr,
		u8 *reg_data,
		u8 cnt) {
	return HAL_I2C_Mem_Read(p_bmp280->i2c_handle, dev_addr << 1U, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, cnt, 100);
}
#elif HAL_SPI_MODULE_ENABLED */
static inline s8 BMP280_SPI_Write (u8 dev_addr,
		u8 reg_addr,
		u8 *reg_data,
		u8 cnt) {
	return 0;
}

static inline s8 BMP280_SPI_Read (u8 dev_addr,
		u8 reg_addr,
		u8 *reg_data,
		u8 cnt) {
	return 0;
}
#endif

#endif /* __BMP280_DRV_H */
