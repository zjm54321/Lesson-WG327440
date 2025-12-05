#pragma once

#include <stdint.h>

// 读取/写入 DS1302 寄存器
void ds1302_write_reg(uint8_t addr, uint8_t data);
uint8_t ds1302_read_reg(uint8_t addr);

// 设置时间（十进制）
void ds1302_set_time(uint8_t hour, uint8_t min, uint8_t sec);

// 读取时间（十进制）
void ds1302_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec);
