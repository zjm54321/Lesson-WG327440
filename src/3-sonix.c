/**
 * @author zjm54321
 * @brief 超声波测距机
 */

#include <utility.h>
#include <stc51.h>
#include <stdint.h>
#include <stdbool.h>
#include <softdelay.h>

#include <i2c.h>
#include <nixie.h>
#include <timer.h>
#include <sonic.h>

typedef enum Display {
    Distance,
    DataShow,
    Settings,
} Display_t;

typedef enum Blindzone {
    Zone0cm,
    Zone10cm,
    Zone20cm,
    Zone30cm,
    Zone40cm,
    Zone50cm,
    Zone60cm,
    Zone70cm,
    Zone80cm,
    Zone90cm,
} Blindzone_t;

typedef struct Setting {
    Display_t display_state;
    Blindzone_t blindzone_state;
    bool sum_action;
} Setting_t;

typedef struct Data {
    uint8_t sum;
    uint8_t last_measure;
    uint8_t this_measure;
    uint8_t measure_ptr;
    uint8_t index_ptr;
    uint8_t measure_arr[10];
} Data_t;

void nixie_display_num(uint8_t position, uint8_t number);
void key_scan(void);

static Data_t sonic_data = {0, 0, 0, 0, {0}};
static Setting_t setting = {Distance, Zone0cm, false};

int32_t main(void) {
    // 关灯
    P25 = 0;
    P26 = 0;
    P27 = 1;
    P0 = 0xFF;
    P27 = 0;

    timer0_init_10ms();
    timer1_init_10ms();
    EA = 1;

    for (;;) {
        key_scan();
    }
}

// 数码管显示
void timer0_isr(void) interrupt(1) {
    switch (setting.display_state) {
    case Distance:
        nixie_display_num(0, sonic_data.this_measure);
        nixie_display_num(3, setting.sum_action ? sonic_data.sum
                                                : sonic_data.last_measure);
        nixie_display(7, setting.sum_action);
        break;
    case DataShow:
        nixie_display_num(5, sonic_data.index_ptr + 1);
        nixie_display_num(0, sonic_data.measure_arr[(sonic_data.measure_ptr -
                                                     sonic_data.index_ptr) %
                                                    10]);
        break;
    case Settings:
        nixie_display_num(0, setting.blindzone_state * 10);
        nixie_display(7, 16);
    default:
        break;
    }
}

inline void nixie_display_num(uint8_t position, uint8_t number) {
    P26 = 1;
    P27 = 1;
    nixie_display(position, number % 10);
    nixie_display(position + 1, (number / 10) % 10);
    nixie_display(position + 2, number / 100);
}

void timer1_isr(void) interrupt(3) {
    switch (setting.display_state) {
    case DataShow:
        P25 = 0;
        P26 = 0;
        P27 = 1;
        P0 = 0b01111111;
        break;
    case Settings:
        P25 = 0;
        P26 = 0;
        P27 = 1;
        P0 = 0b10111111;
        break;
    default:
        P25 = 0;
        P26 = 0;
        P27 = 1;
        P0 = 0xFF;
        break;
    }
}

inline void key_scan(void) {
    KEY_PRESSED(P33, {
        sonic_data.last_measure = sonic_data.this_measure;
        sonic_data.this_measure = csb_measure(setting.blindzone_state);
        sonic_data.sum = sonic_data.last_measure + sonic_data.this_measure;
        sonic_data.measure_ptr = (sonic_data.measure_ptr + 1) % 10;
        sonic_data.measure_arr[sonic_data.measure_ptr] =
            sonic_data.this_measure;

        for (uint8_t i = 0; i < 10; i++) {
            EA = 0;
            P25 = 0;
            P26 = 0;
            P27 = 1;
            P0 = 0b11111110;
            EA = 1;
            delay20ms();
            EA = 0;
            P25 = 0;
            P26 = 0;
            P27 = 1;
            P0 = 0b11111111;
            EA = 1;
            delay20ms();
        }
    });
    // 设置时间
    KEY_PRESSED(P32, {
        if (setting.display_state == DataShow)
            setting.display_state = Distance;
        else
            setting.display_state = DataShow;
    });
    KEY_PRESSED(P31, {
        if (setting.display_state == Settings)
            setting.display_state = Distance;
        else
            setting.display_state = Settings;
    });
    KEY_PRESSED(P30, {
        switch (setting.display_state) {
        case Distance:
            setting.sum_action = !setting.sum_action;
            break;
        case DataShow:
            sonic_data.index_ptr = (sonic_data.index_ptr + 1) % 10;
            break;
        case Settings:
            setting.blindzone_state = (setting.blindzone_state + 1) % 10;
            break;
        default:
            break;
        }
    });
}