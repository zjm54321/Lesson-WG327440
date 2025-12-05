/**
 * @file         i2c.h
 * @brief        51单片机通过I0模拟I2C总线
 * @author       GhpZhu,zjm54321
 * @date         2025.11.17
 */

#pragma once
#include <stdint.h>

void i2c_delay(uint8_t i);
void i2c_start(void);
void i2c_stop(void);

void i2c_sendbyte(uint8_t byt);
uint8_t i2c_waitack(void);
uint8_t i2c_receivebyte(void);
void i2c_sendack(uint8_t ackbit);

void write_eeprom(uint8_t add, uint8_t val);
uint8_t read_eeprom(uint8_t add);

void init_pcf8591(void);
uint8_t adc_pcf8591(uint8_t adc_addr);
