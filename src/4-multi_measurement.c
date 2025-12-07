/**
 * @author zjm54321
 * @brief 多功能测量仪表
 */

#include <utility.h>
#include <stc51.h>
#include <stdint.h>
#include <stdbool.h>

#include <timer.h>
#include <nixie.h>
#include <onewire.h>
#include <i2c.h>

#define SAMPLE_COUNTS 20        // 采样分频数：Timer0 每计多少个数中断一次
#define PCA_CLOCK_FREQ 921600UL // 晶振  11.0592MHz

typedef enum Datashow {
    VoltageShow,
    FrequencyShow,
    TemperatureShow,
} Datashow_t;

typedef enum Display {
    Data,
    Trace,
    Threshold,
} Display_t;

typedef enum OneWireAction {
    Stage1Ready,
    Stage1Done,
    Stage2Ready,
    Stage2Done,
} OneWireAction_t;

typedef struct Settings {
    Datashow_t datashow;
    Display_t display;
    uint8_t threshold_value; // 2-50 代表0.2V-5V
    bool alarming;
} Settings_t;

typedef struct Data {
    uint8_t voltage;
    uint8_t voltage_last;
    uint16_t temperature;
    uint16_t temperature_last;
    uint32_t ne555_freq;
    uint32_t ne555_freq_last;
} Data_t;

void ne555_init(void);
void nixie_display_num(uint8_t position, uint8_t number);
void read_onewire(void);
void key_scan(void);
void led_config(void);

// 定义全局变量
static Settings_t settings = {
    .datashow = VoltageShow,
    .display = Data,
    .threshold_value = 50,
    .alarming = false,
};
static volatile Data_t data_value = {
    .voltage = 0,
    .voltage_last = 0,
    .temperature = 0,
    .temperature_last = 0,
    .ne555_freq = 0,
    .ne555_freq_last = 0,
};
static volatile OneWireAction_t onewire_state = Stage1Ready;

int32_t main(void) {
    // 关灯
    P25 = 0;
    P26 = 0;
    P27 = 1;
    P0 = 0xFF;

    ne555_init();
    timer1_init_10ms();
    timer2_init_1ms();
    EA = 1;

    for (;;) {
        key_scan();
        read_onewire();
        // led_config();
        settings.alarming =
            (data_value.voltage < settings.threshold_value) ? false : true;
    }
}

// 显示
void timer1_isr(void) interrupt(3) {
    switch (settings.display) {
    case Data:
        switch (settings.datashow) {
        case VoltageShow:
            nixie_display_u8(data_value.voltage);
            nixie_display(1, 10);
            nixie_display(7, 20);
            break;
        case FrequencyShow:
            uint8_t freq_high = data_value.ne555_freq / 1000;
            uint8_t freq_low = data_value.ne555_freq % 1000;
            nixie_display_num(0, freq_high);
            nixie_display_num(3, freq_low);
            nixie_display(7, 16);
            break;
        case TemperatureShow:
            uint8_t temp_high = data_value.temperature / 1000;
            uint8_t temp_low = data_value.temperature % 1000;
            nixie_display_num(3, temp_high);
            nixie_display_num(0, temp_low);
            nixie_display(2, 10);
            nixie_display(7, 13);
            break;
        }
        break;
    case Trace:
        switch (settings.datashow) {
        case VoltageShow:
            nixie_display_u8(data_value.voltage_last);
            nixie_display(1, 10);
            nixie_display(7, 17);
            nixie_display(6, 20);
            break;
        case FrequencyShow:
            uint8_t freq_high = data_value.ne555_freq_last / 1000;
            uint8_t freq_low = data_value.ne555_freq_last % 1000;
            nixie_display_num(0, freq_high);
            nixie_display_num(3, freq_low);
            nixie_display(7, 17);
            nixie_display(6, 16);
            break;
        case TemperatureShow:
            uint8_t temp_high = data_value.temperature_last / 1000;
            uint8_t temp_low = data_value.temperature_last % 1000;
            nixie_display_num(3, temp_high);
            nixie_display_num(0, temp_low);
            nixie_display(2, 10);
            nixie_display(7, 17);
            nixie_display(6, 13);
            break;
        }
        break;
    case Threshold:
        nixie_display_u8(settings.threshold_value);
        nixie_display(1, 10);
        nixie_display(7, 21);
        break;
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
// NE555 频率处理
inline void ne555_init(void) {
    // 1. 初始化 PCA (作为 16位 自由跑秒表)
    CMOD = 0x00;
    CL = 0;
    CH = 0;
    CR = 1;

    // 2. 初始化 Timer0 (作为分频计数器)
    AUXR &= 0x7F;
    TMOD &= 0xF0;
    TMOD |= 0x06;
    TL0 = (uint8_t)(256 - SAMPLE_COUNTS);
    TH0 = (uint8_t)(256 - SAMPLE_COUNTS);
    TF0 = 0;
    ET0 = 1;
    TR0 = 1;
}

void timer0_isr(void) interrupt(1) {
    static uint16_t last_time = 0;
    uint16_t current_time;
    uint16_t delta_time;

    // 1. 能够进中断，说明已经过了 SAMPLE_COUNTS 个脉冲
    // 2. 读取 PCA 时间 (处理 16位 读取原子性问题)
    // 先读低位，再读高位
    current_time = CL;
    current_time |= (uint16_t)(CH << 8);
    // 3. 计算时间差 (利用无符号整数溢出回绕特性，直接相减即可)
    // 例如: current=10, last=65500 -> 10 - 65500 = 46 (在uint16下)
    delta_time = current_time - last_time;
    // 4. 更新基准
    last_time = current_time;

    if (delta_time != 0)
        data_value.ne555_freq =
            ((uint32_t)SAMPLE_COUNTS * PCA_CLOCK_FREQ) / delta_time;
    else
        data_value.ne555_freq = UINT32_MAX;
}
// 读取电压和温度,设置 Led
void timer2_isr(void) interrupt(12) {
    static uint8_t count = 0;
    static uint16_t temp_cnt = 0;
    static uint8_t light_count = 0;
    if (++count > 20) {
        count = 0;
        // data_value.voltage_last = data_value.voltage;
        data_value.voltage =
            (uint16_t)(((float)adc_pcf8591(0x03) / 255.0f * 5.0f) * 10) % 100;
    }

    temp_cnt++;
    if (temp_cnt >= 100 && onewire_state == Stage2Done) {
        temp_cnt = 0;
        onewire_state = Stage1Ready;
    }
    if (temp_cnt >= 750 && onewire_state == Stage1Done) {
        temp_cnt = 0;
        onewire_state = Stage2Ready;
    }

    if (settings.alarming) {
        static uint8_t count = 0;
        if (++count >= 200) {
            count = 0;
            P25 = 0;
            P26 = 0;
            P27 = 1;
            P0 &= 0b00000111;
        }
        if (count == 100) {
            P25 = 0;
            P26 = 0;
            P27 = 1;
            P0 |= 0b11111000;
        }
    } else {
        P25 = 0;
        P26 = 0;
        P27 = 1;
        P0 |= 0b11111000;
    }

    P25 = 0;
    P26 = 0;
    P27 = 1;
    P00 = 1;
    P01 = 1;
    P02 = 1;
    switch (settings.datashow) {
    case VoltageShow:
        P00 = 0;
        break;
    case FrequencyShow:
        P01 = 0;
        break;
    case TemperatureShow:
        P02 = 0;
        break;
    }
}

inline void read_onewire(void) {
    switch (onewire_state) {
    case Stage1Ready:
        EA = 0;
        init_ds18b20();
        Write_DS18B20(0xcc);
        Write_DS18B20(0x44);
        onewire_state = Stage1Done;
        EA = 1;
        break;
    case Stage2Ready:
        EA = 0;
        init_ds18b20();
        Write_DS18B20(0xcc);
        Write_DS18B20(0xbe);
        uint8_t LSB = Read_DS18B20();
        uint8_t MSB = Read_DS18B20();
        uint16_t temp = (MSB << 8) | LSB;
        onewire_state = Stage2Done;
        EA = 1;
        data_value.temperature = (uint16_t)((float)temp * 0.0625f * 100);
        break;
    default:
        break;
    }
}

inline void key_scan(void) {
    KEY_PRESSED(P33, { settings.datashow = (settings.datashow + 1) % 3; });
    KEY_PRESSED(P32, {
        data_value.voltage_last = data_value.voltage;
        data_value.temperature_last = data_value.temperature;
        data_value.ne555_freq_last = data_value.ne555_freq;
    });
    KEY_PRESSED(P31, { settings.display = (settings.display + 1) % 3; });
    KEY_PRESSED(P30, {
        // settings.threshold_value在2-50之间，步长为2
        settings.threshold_value += 2;
        if (settings.threshold_value > 50)
            settings.threshold_value = 2;
    });
}