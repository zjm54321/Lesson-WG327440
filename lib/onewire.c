#include <stc51.h>
#include <stdint.h>
#include <stdbool.h>

#define DQ P14

// 单总线延时函数
void Delay_OneWire(unsigned int t) // STC89C52RC
{
    while (t--)
        ;
}

// 通过单总线向DS18B20写一个字节
void Write_DS18B20(uint8_t dat) {
    for (uint8_t i = 0; i < 8; i++) {
        DQ = 0;
        DQ = dat & 0x01;
        Delay_OneWire(50);
        DQ = 1;
        dat >>= 1;
    }
    Delay_OneWire(50);
}

// 从DS18B20读取一个字节
uint8_t Read_DS18B20(void) {
    uint8_t dat = 0;

    for (uint8_t i = 0; i < 8; i++) {
        DQ = 0;
        dat >>= 1;
        DQ = 1;
        if (DQ) {
            dat |= 0x80;
        }
        Delay_OneWire(50);
    }
    return dat;
}

// DS18B20设备初始化
bool init_ds18b20(void) {
    bool initflag = 0;

    DQ = 1;
    Delay_OneWire(120);
    DQ = 0;
    Delay_OneWire(800);
    DQ = 1;
    Delay_OneWire(100);
    initflag = DQ;
    Delay_OneWire(50);

    return initflag;
}
