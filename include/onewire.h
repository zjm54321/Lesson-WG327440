#pragma once

#include <stdint.h>
#include <stdbool.h>

bool init_ds18b20(void);
uint8_t Read_DS18B20(void);
void Write_DS18B20(uint8_t dat);
