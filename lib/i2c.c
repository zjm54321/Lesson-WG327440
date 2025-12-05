#include <stc51.h>
#include <compiler.h>
#include <stdint.h>
#include <stddef.h>

#define DELAY_TIME 5

/** 定义I2C总线时钟线和数据线 */
#define scl P20
#define sda P21

/**I2C总线中一些必要的延时*/
inline void i2c_delay(uint8_t i) {
    do {
        NOP();
    } while (i--);
}

/** @brief 产生I2C总线启动条件.*/
inline void i2c_start(void) {
    sda = 1;
    scl = 1;
    i2c_delay(DELAY_TIME);
    sda = 0;
    i2c_delay(DELAY_TIME);
    scl = 0;
}

/**@brief 产生I2C总线停止条件*/
inline void i2c_stop(void) {
    sda = 0;
    scl = 1;
    i2c_delay(DELAY_TIME);
    sda = 1;
    i2c_delay(DELAY_TIME);
}

/**I2C发送一个字节的数据*/
void i2c_sendbyte(uint8_t byt) {
    EA = 0;
    for (uint8_t i = 0; i < 8; i++) {
        scl = 0;
        i2c_delay(DELAY_TIME);
        if (byt & 0x80) {
            sda = 1;
        } else {
            sda = 0;
        }
        i2c_delay(DELAY_TIME);
        scl = 1;
        byt <<= 1;
        i2c_delay(DELAY_TIME);
    }
    EA = 1;
    scl = 0;
}

/**等待应答*/
uint8_t i2c_waitack(void) {
    uint8_t ackbit = 0;

    scl = 1;
    i2c_delay(DELAY_TIME);
    ackbit = sda; // while(sda);  //wait ack
    scl = 0;
    i2c_delay(DELAY_TIME);

    return ackbit;
}

/**I2C接收一个字节数据*/
uint8_t i2c_receivebyte(void) {
    uint8_t da = 0;
    EA = 0;
    for (uint8_t i = 0; i < 8; i++) {
        scl = 1;
        i2c_delay(DELAY_TIME);
        da <<= 1;
        if (sda)
            da |= 0x01;
        scl = 0;
        i2c_delay(DELAY_TIME);
    }
    EA = 1;
    //
    return da;
}

/**发送应答*/
void i2c_sendack(uint8_t ackbit) {
    scl = 0;
    sda = ackbit; // 0：应答；1：非应答
    i2c_delay(DELAY_TIME);
    scl = 1;
    i2c_delay(DELAY_TIME);
    scl = 0;
    sda = 1;
    i2c_delay(DELAY_TIME);
}

uint8_t adc_pcf8591(uint8_t adc_addr) {
    uint8_t adc_data = 0;

    // 主设备给从设备发送/写入数据
    i2c_start();
    i2c_sendbyte(0x90); // 写地址
    i2c_waitack();
    i2c_sendbyte(0x40 | adc_addr); // 控制字节
    i2c_waitack();

    // 主设备给从设备接收/读取数据
    i2c_start();
    i2c_sendbyte(0x91); // 读地址
    i2c_waitack();
    adc_data = i2c_receivebyte();
    i2c_sendack(1);
    i2c_stop();

    return adc_data;
}