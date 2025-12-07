#pragma once
#include <clangd.h>
#include <keil.h>

#include <softdelay.h>

// 带消抖的按键处理：pin 为引脚，BODY 为按键确认后的业务逻辑
#define KEY_PRESSED(pin, BODY)                                                 \
    do {                                                                       \
        if ((pin) == 0) {                                                      \
            delay10ms();                                                       \
            if ((pin) == 0) {                                                  \
                BODY while ((pin) == 0);                                       \
            }                                                                  \
        }                                                                      \
    } while (0)
