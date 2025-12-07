#include <stc51.h>
#include <compiler.h>
#include <stdbool.h>
#include <stdint.h>

#define Tx P10 // 超声波发送引脚
#define Rx P11 // 超声波接收引脚

// ================= 延时函数 =================
// 产生 40kHz 半周期 (约 12.5us)
// 请根据你的实际晶振频率微调这里的 _nop_
inline void delay13us(void) {
    NOP();
    NOP();
    uint8_t i = 33;
    while (--i)
        ;
}

// 盲区延时 (588us)
inline void delay10cm(void) {
    NOP();
    NOP();
    NOP();
    uint8_t i = 7;
    uint8_t j = 79;
    do {
        while (--j)
            ;
    } while (--i);
}

// ================= PCA 初始化 =================
inline void pca_init(void) {
    // CMOD: CIDL=0, CPS=000 (系统时钟/12)
    // 假设 11.0592MHz，计数频率约 0.92MHz (1us/次)
    CMOD = 0x00;
    // 2. 初始关闭所有捕获模式，防止误触
    CCAPM0 = 0x00;
    // 3. 确保计时器关闭
    CR = 0;
}

// 测量距离，最大2米
uint8_t csb_measure(uint8_t delay) {
    // 发送
    EA = 0; // 关总中断
    for (uint8_t i = 0; i < 10; i++) {
        Tx = 1;
        delay13us();
        Tx = 0;
        delay13us();
    }
    // 发送完毕
    // 接受
    CL = 0;
    CH = 0;
    CR = 1;

    // 3. 盲区延时
    for (uint8_t i = 0; i < delay; i++)
        delay10cm();

    // 4. 开启捕获
    CCF0 = 0;
    CCAPM0 = 0x10;

    // 5. 等待回波 (最大 2 米)
    while (!CCF0)
        if (CH > 0x30)
            break;

    // 6. 停止与计算
    CR = 0;
    CCAPM0 = 0x00;

    EA = 1; // 开总中断

    if (CCF0) {
        CCF0 = 0;
        uint16_t time = (CCAP0H << 8) | CCAP0L;
        uint16_t distance = time * 17 / 1000;
        return (uint8_t)distance;
    } else
        return UINT8_MAX;
}