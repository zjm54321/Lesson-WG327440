/**
 * @author zjm54321
 * @brief 电子钟
 */

#include <utility.h>
#include <stc51.h>
#include <stdint.h>
#include <stdbool.h>
#include <softdelay.h>

#include <i2c.h>
#include <nixie.h>
#include <timer.h>
#include <ds1302.h>

typedef enum State {
    Setting,
    Stopwatch,
    Clock,
    Alarming,
} State_t;

typedef enum SettingState {
    SetSecondsUnits,
    SetSecondsTens,
    SetMinutesUnits,
    SetMinutesTens,
    SetHoursUnits,
    SetHoursTens,
} SettingState_t;

typedef struct Time {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} Time_t;

void nixie_display_setting(Time_t time, SettingState_t setting_state);
void nixie_display_clock(Time_t time);
void alarming_off(void);

static volatile State_t state = Setting;
static volatile SettingState_t setting_state = SetSecondsUnits;
static volatile Time_t time_count = {0, 0, 0};
static volatile Time_t alarm_time = {0, 0, 0};

uint32_t main(void) {

    timer0_init_10ms();
    timer1_init_10ms();
    EA = 1; // 全局中断使能

    for (;;) {
        // 选择位
        KEY_PRESSED(P33, {
            if (state == Setting) {
                if (setting_state == SetHoursTens)
                    setting_state = SetSecondsUnits;
                else
                    setting_state++;
            } else if (state == Alarming) {
                alarming_off();
            } else {
                state = Setting;
            }
        });
        // 设置时间
        KEY_PRESSED(P32, {
            if (state == Setting) {
                switch (setting_state) {
                case SetSecondsUnits:
                    time_count.seconds = (time_count.seconds / 10) * 10 +
                                         (time_count.seconds + 1) % 10;
                    break;
                case SetSecondsTens:
                    time_count.seconds =
                        (time_count.seconds % 10) +
                        (((time_count.seconds / 10) + 1) % 6) * 10;
                    break;
                case SetMinutesUnits:
                    time_count.minutes = (time_count.minutes / 10) * 10 +
                                         (time_count.minutes + 1) % 10;
                    break;
                case SetMinutesTens:
                    time_count.minutes =
                        (time_count.minutes % 10) +
                        (((time_count.minutes / 10) + 1) % 6) * 10;
                    break;
                case SetHoursUnits:
                    time_count.hours = (time_count.hours / 10) * 10 +
                                       (time_count.hours + 1) % 10;
                    if (time_count.hours >= 24)
                        time_count.hours = time_count.hours % 10;
                    break;
                case SetHoursTens:
                    time_count.hours = (time_count.hours % 10) +
                                       (((time_count.hours / 10) + 1) % 3) * 10;
                    if (time_count.hours >= 24)
                        time_count.hours = time_count.hours % 10;
                    break;
                }
            } else if (state == Alarming) {
                alarming_off();
            }
        });
        KEY_PRESSED(P31, {
            if (state == Setting) {
                // 完成设置，写入 DS1302
                ds1302_set_time(time_count.hours, time_count.minutes,
                                time_count.seconds);
                state = Stopwatch;
            } else if (state == Alarming) {
                alarming_off();
            }
        });
        KEY_PRESSED(P30, {
            if (state == Setting) {
                alarm_time = time_count;
                state = Clock;
            } else if (state == Alarming) {
                alarming_off();
            }
        });
        if (state == Alarming) {
            EA = 0;
            P2 = (P2 & 0x1F) | 0xA0;
            P06 = 1;

            P2 = ((P2 & 0x1f) | 0x80);
            P0 = 0x00;
            delay500ms();
            P2 = ((P2 & 0x1f) | 0x80);
            P0 = 0xFF;
            delay500ms();
        }
    }
}

// 用作时间显示
void timer0_isr(void) interrupt(1) {
    static uint8_t count = 0;
    static bool blink_state = false;

    if (state == Setting) {
        // 设置位闪烁
        if (++count >= 100) {
            count = 0;
            blink_state = !blink_state;
        }
        if (blink_state)
            nixie_display_clock(time_count);
        else
            nixie_display_setting(time_count, setting_state);
    } else {
        nixie_display_clock(time_count);
    }
}

inline void nixie_display_clock(Time_t time) {
    nixie_display_time(time.hours, time.minutes, time.seconds);
}

// 显示除了正在设置的位以外的时间
inline void nixie_display_setting(Time_t time, SettingState_t setting_state) {
    P26 = 1;
    P27 = 1;
    if (setting_state != SetSecondsUnits)
        nixie_display(0, time.seconds % 10);
    if (setting_state != SetSecondsTens)
        nixie_display(1, time.seconds / 10);
    nixie_display(2, 23); // 显示'-'
    if (setting_state != SetMinutesUnits)
        nixie_display(3, time.minutes % 10);
    if (setting_state != SetMinutesTens)
        nixie_display(4, time.minutes / 10);
    nixie_display(5, 23); // 显示'-'
    if (setting_state != SetHoursUnits)
        nixie_display(6, time.hours % 10);
    if (setting_state != SetHoursTens)
        nixie_display(7, time.hours / 10);
}

void timer1_isr(void) interrupt(3) {
    static uint8_t count = 0;
    if (++count >= 20) {
        count = 0;
        // 每200ms读取一次时间
        if (state != Setting) {
            ds1302_get_time(&time_count.hours, &time_count.minutes,
                            &time_count.seconds);
        }
        if (state == Clock) {
            // 检查闹钟
            if (time_count.hours == alarm_time.hours &&
                time_count.minutes == alarm_time.minutes &&
                time_count.seconds == alarm_time.seconds) {
                state = Alarming;
            }
        }
    }
}

void alarming_off(void) {
    state = Setting;
    setting_state = SetSecondsUnits;
    time_count.hours = 0;
    time_count.minutes = 0;
    time_count.seconds = 0;

    P2 = (P2 & 0x1F) | 0xA0;
    P06 = 0;

    P2 = ((P2 & 0x1f) | 0x80);
    P0 = 0xFF;

    EA = 1;
}