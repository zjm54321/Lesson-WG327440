#pragma once
#include <stc51.h>

void exti_int0_init(void) {
    IT0 = 0; // INT0(P3.2)上升沿+下降沿中断
    EX0 = 1; // 使能INT0中断
}

void exti_int1_init(void) {
    IT1 = 0; // INT1(P3.3)上升沿+下降沿中断
    EX1 = 1; // 使能INT1中断
}