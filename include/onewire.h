#pragma once

#include <stdint.h>
#include <stdbool.h>

static void Delay_OneWire(uint16_t t);
bool init_ds18b20(void);
void Write_DS18B20(uint8_t dat);
uint8_t Read_DS18B20(void);
float rd_temperature_f(void);
