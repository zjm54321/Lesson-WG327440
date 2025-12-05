#include <stc51.h>

void serial1_init_timer1(void) {
    SCON = 0x50;  // 8位数据,可变波特率
    AUXR |= 0x40; // 定时器时钟1T模式
    AUXR &= 0xFE; // 串口1选择定时器1为波特率发生器
    TMOD &= 0x0F; // 设置定时器模式
    TL1 = 0xE0;   // 设置定时初始值
    TH1 = 0xFE;   // 设置定时初始值
    ET1 = 0;      // 禁止定时器中断
    TR1 = 1;      // 定时器1开始计时
    ES = 1;       // 使能串口1中断
}

void serial1_init_timer2(void) {
    SCON = 0x50;  // 8位数据,可变波特率
    AUXR |= 0x01; // 串口1选择定时器2为波特率发生器
    AUXR |= 0x04; // 定时器时钟1T模式
    T2L = 0xE0;   // 设置定时初始值
    T2H = 0xFE;   // 设置定时初始值
    AUXR |= 0x10; // 定时器2开始计时
    ES = 1;       // 使能串口1中断
}
