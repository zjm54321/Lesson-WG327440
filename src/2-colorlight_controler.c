/**
 * @author zjm54321
 * @brief 彩灯控制器
 */

#include <utility.h>
#include <stc51.h>
#include <stdint.h>
#include <stdbool.h>
#include <softdelay.h>

#include <i2c.h>
#include <nixie.h>
#include <timer.h>

// 定义结构

// 工作模式
typedef enum ModeState {
    Mode1, // 按照 L1-L8 的顺序，从左到右单循环点亮
    Mode2, // 按照 L1-L8 的顺序，从右到左单循环点亮
    Mode3, // 按照 L1L8 、 L2L7 、 L3L6 和 L4L5 的顺序循环点亮
    Mode4, // 按照 L4L5 、 L3L6 、 L2L7 和 L1L8 的顺序循环点亮
} ModeState_t;
typedef struct Mode {
    ModeState_t mode_state;
    uint8_t position;
} Mode_t;
const uint8_t ModeArr[4][8] = {
    {0b11111110, 0b11111101, 0b11111011, 0b11110111, 0b11101111, 0b11011111,
     0b10111111, 0b01111111}, // Mode1
    {0b01111111, 0b10111111, 0b11011111, 0b11101111, 0b11110111, 0b11111011,
     0b11111101, 0b11111110},                         // Mode2
    {0b01111110, 0b10111101, 0b11011011, 0b11100111}, // Mode3
    {0b11100111, 0b11011011, 0b10111101, 0b01111110}, // Mode4
};

// 亮度等级
typedef enum LightState {
    Low,
    Medium,
    High,
    VeryHigh,
} LightState_t;

// 流转间隔
typedef enum Duration {
    Duration400ms = 4,
    Duration500ms = 5,
    Duration600ms = 6,
    Duration700ms = 7,
    Duration800ms = 8,
    Duration900ms = 9,
    Duration1000ms = 10,
    Duration1100ms = 11,
    Duration1200ms = 12,
} Duration_t;

// 亮度
typedef struct Light {
    Mode_t mode;
    LightState_t light_state;
    Duration_t duration;
    uint8_t value;
} Light_t;
// 设置
typedef struct Settings {
    bool setting_on;
    // 运行模式 & 流转间隔
    bool setting_mode;
    bool nixe_display_light;
    bool start;
} Settings_t;

// 函数定义
void key_scan(void);

// 定义全局变量
static volatile Light_t light = {
    .mode = {.mode_state = Mode1, .position = 0},
    .light_state = Low,
    .duration = Duration400ms,
    .value = 0xFF,
};
static Settings_t settings = {
    .start = false,
    .setting_on = false,
    .setting_mode = false,
    .nixe_display_light = false,

};

int32_t main(void) {
    timer0_init_10ms();
    timer1_init_10ms();
    timer2_init_1ms();
    EA = 1; // 全局中断使能

    for (;;) {
        key_scan();
        settings.nixe_display_light = false;
    }
}

// 控制数码管显示状态
void timer0_isr(void) interrupt(1) {
    static uint8_t count = 0;
    static bool blink_state = false;

    if (settings.setting_on) {
        if (++count >= 80) {
            count = 0;
            blink_state = !blink_state;
        }

        P26 = 1;
        P27 = 1;
        nixie_display(4, 23); // 显示'-'
        nixie_display(7, 23); // 显示'-'
        if (blink_state) {
            if (!settings.setting_mode) {
                nixie_display(6, light.mode.mode_state + 1);
            } else {
                uint16_t d = light.duration * 100;
                nixie_display(3, d / 1000);
                nixie_display(2, (d / 100) % 10);
                nixie_display(1, (d / 10) % 10);
                nixie_display(0, d % 10);
            }
        } else {
            uint16_t d = light.duration * 100;
            nixie_display(6, light.mode.mode_state + 1);
            nixie_display(3, d / 1000);
            nixie_display(2, (d / 100) % 10);
            nixie_display(1, (d / 10) % 10);
            nixie_display(0, d % 10);
        }
    } else if (settings.nixe_display_light) {
        P26 = 1;
        P27 = 1;
        nixie_display(7, 23);                    // 显示'-'
        nixie_display(6, light.light_state + 1); // 显示'-'
    }
}

// 控制 Led 灯显示状态 & 亮度
void timer1_isr(void) interrupt(3) {
    static uint8_t cnt = 0;
    if (++cnt >= 50) {
        cnt = 0;
        uint8_t adc_value = adc_pcf8591(0x03); // 读取ADC值（Rb2）
        adc_value /= 64;                       // 转换为0-3范围内的数值
        light.light_state = (LightState_t)adc_value;
    }

    static uint8_t count = 0;
    if (!settings.start)
        return;
    if (++count >= light.duration * 10) {
        count = 0;

        switch (light.mode.mode_state) {
        case Mode1:
        case Mode2:
            light.mode.position = (light.mode.position + 1) % 8;
            break;
        case Mode3:
        case Mode4:
            light.mode.position = (light.mode.position + 1) % 4;
            break;
        default:
            light.value = 0xFF;
            return;
        }

        light.value = ModeArr[light.mode.mode_state][light.mode.position];
    }
}

// 显示 Led 灯
void timer2_isr(void) interrupt(12) {
    static uint8_t count = 0;
    count = (count + 1) % 4;

    P25 = 0;
    P26 = 0;
    P27 = 1;
    P0 = (count <= light.light_state) ? light.value : 0xFF;
    P27 = 0;
    P0 = 0xFF;
}

inline void key_scan(void) {
    // S5 控制开始/暂停
    KEY_PRESSED(P32, {
        if (settings.setting_on) {
            settings.setting_on = false;
            settings.start = true;
        } else {
            settings.start = !settings.start;
        }
    });
    // S4 控制设置模式切换
    KEY_PRESSED(P33, {
        if (!settings.setting_on) {
            settings.setting_on = true;
            settings.start = false;
            light.value = 0xFF;
        } else {
            settings.setting_mode = !settings.setting_mode;
        }
    });
    // S7 加按键
    KEY_PRESSED(P30, {
        if (settings.setting_on) {
            if (settings.setting_mode) {
                // 切换运行模式
                light.mode.mode_state = (light.mode.mode_state + 1) % 4;
                light.mode.position = 0;
            } else {
                // 切换流转间隔
                light.duration = (light.duration == Duration1200ms)
                                     ? Duration400ms
                                     : (Duration_t)(light.duration + 1);
            }
        } else {
            settings.nixe_display_light = true;
        }
    });
    // S6 减按键
    KEY_PRESSED(P31, {
        if (settings.setting_on) {
            if (settings.setting_mode) {
                // 切换运行模式
                light.mode.mode_state =
                    (light.mode.mode_state == Mode1)
                        ? Mode4
                        : (ModeState_t)(light.mode.mode_state - 1);
                light.mode.position = 0;
            } else {
                // 切换流转间隔
                light.duration = (light.duration == Duration400ms)
                                     ? Duration1200ms
                                     : (Duration_t)(light.duration - 1);
            }
        }
    });
}