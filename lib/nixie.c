#include <stc51.h>
#include <stdint.h>
#include <softdelay.h>

// --- 数码管 0-9 段码表 (共阴) ---
const uint8_t nixie_table[] = {
    0xC0, // "0"
    0xF9, // "1"
    0xA4, // "2"
    0xB0, // "3"
    0x99, // "4"
    0x92, // "5"
    0x82, // "6"
    0xF8, // "7"
    0x80, // "8"
    0x90, // "9"
    0x7F, // "."
    0x88, // "A"
    0x83, // "B"
    0xC6, // "C"
    0xA1, // "D"
    0x86, // "E"
    0x8E, // "F"
    0x89, // "H"
    0xC7, // "L"
    0xC8, // "n"
    0xC1, // "u"
    0x8C, // "P"
    0xA3, // "o"
    0xBF, // "-"
    0xFF  // 熄灭
};

void nixie_display(uint8_t location, uint8_t digit) {
    // 选择位置
    P25 = 1;
    P0 = 0xFF;
    P25 = 0;
    P0 = 0x80 >> location;
    P25 = 1;
    // 显示数字
    P0 = nixie_table[digit];
    delay1ms();
    P0 = 0xFF;
}

void nixie_display_u8(uint8_t number) {
    P26 = 1;
    P27 = 1;
    nixie_display(0, number % 10);
    nixie_display(1, (number / 10) % 10);
    nixie_display(2, number / 100);
}

void nixie_display_fpu8(uint16_t number) {
    P26 = 1;
    P27 = 1;
    nixie_display(2, 10); // 显示小数点
    nixie_display(0, number % 10);
    nixie_display(1, (number / 10) % 10);
    nixie_display(2, number / 100);
}

void nixie_display_stopwatch(uint8_t minutes, uint8_t seconds) {
    P26 = 1;
    P27 = 1;
    nixie_display(0, seconds % 10);
    nixie_display(1, seconds / 10);
    nixie_display(2, 23); // 显示'-'
    nixie_display(3, minutes % 10);
    nixie_display(4, minutes / 10);
}