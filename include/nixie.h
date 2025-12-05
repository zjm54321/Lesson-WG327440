#pragma once

#include <stdint.h>

extern const uint8_t nixie_table[];
void nixie_display(uint8_t location, uint8_t digit);
void nixie_display_u8(uint8_t number);
void nixie_display_fpu8(uint16_t number);
void nixie_display_stopwatch(uint8_t minutes, uint8_t seconds);