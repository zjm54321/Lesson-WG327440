#include <stc51.h>
#include <compiler.h>
#include <stdint.h>

// 引脚定义（根据原文件：SCK=P1^7, SDA=P2^3, RST=P1^3）
#define DS1302_SCK P17
#define DS1302_SDA P23
#define DS1302_RST P13

// DS1302 最小延时（按需要可调整）
#define DS1302_DELAY_TIME 1

static inline void ds1302_delay(uint8_t i) {
	do {
		NOP();
	} while (i--);
}

// 写一个字节到 DS1302，低位在前
static void ds1302_write_byte(uint8_t val) {
	for (uint8_t i = 0; i < 8; i++) {
		DS1302_SCK = 0;
		ds1302_delay(DS1302_DELAY_TIME);

		DS1302_SDA = (val & 0x01);
		val >>= 1;

		ds1302_delay(DS1302_DELAY_TIME);
		DS1302_SCK = 1;
		ds1302_delay(DS1302_DELAY_TIME);
	}
}

// 从 DS1302 读一个字节，低位在前
static uint8_t ds1302_read_byte(void) {
	uint8_t val = 0;

	for (uint8_t i = 0; i < 8; i++) {
		DS1302_SCK = 0;
		ds1302_delay(DS1302_DELAY_TIME);

		val >>= 1;
		if (DS1302_SDA) {
			val |= 0x80;
		}

		DS1302_SCK = 1;
		ds1302_delay(DS1302_DELAY_TIME);
	}

	return val;
}

// 向 DS1302 指定寄存器写一个字节（关闭中断以免打断时序）
void ds1302_write_reg(uint8_t addr, uint8_t data) {

	DS1302_RST = 0;
	NOP();
	DS1302_SCK = 0;
	NOP();
	DS1302_RST = 1;
	NOP();

	ds1302_write_byte(addr); // 地址（写命令）
	ds1302_write_byte(data); // 数据

	DS1302_RST = 0;
}

// 从 DS1302 指定寄存器读一个字节
uint8_t ds1302_read_reg(uint8_t addr) {
	uint8_t val = 0;

	DS1302_RST = 0;
	NOP();
	DS1302_SCK = 0;
	NOP();
	DS1302_RST = 1;
	NOP();

	ds1302_write_byte(addr); // 地址（读命令）
	val = ds1302_read_byte();

	DS1302_RST = 0;
	NOP();
	DS1302_SCK = 0;
	NOP();
	DS1302_SCK = 1;
	NOP();
	DS1302_SDA = 0;
	NOP();
	DS1302_SDA = 1;
	NOP();

	return val;
}

// 设置时间（传入十进制数值）
void ds1302_set_time(uint8_t hour, uint8_t min, uint8_t sec) {
	// 关闭写保护
	ds1302_write_reg(0x8e, 0x00);

	// 按 BCD 写入时分秒
	ds1302_write_reg(0x84, (uint8_t)(hour / 10 * 16 + hour % 10));
	ds1302_write_reg(0x82, (uint8_t)(min / 10 * 16 + min % 10));
	ds1302_write_reg(0x80, (uint8_t)(sec / 10 * 16 + sec % 10));

	// 打开写保护
	ds1302_write_reg(0x8e, 0x80);
}

// 读取时间（返回十进制数值）
void ds1302_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec) {
	uint8_t bcd_h, bcd_m, bcd_s;

	// 直接从寄存器读取 BCD 数据
	bcd_h = ds1302_read_reg(0x85); // 小时寄存器（读地址）
	bcd_m = ds1302_read_reg(0x83); // 分钟寄存器（读地址）
	bcd_s = ds1302_read_reg(0x81); // 秒寄存器（读地址）

	// BCD -> 十进制
	if (hour) {
		*hour = (uint8_t)((bcd_h >> 4) * 10 + (bcd_h & 0x0F));
	}
	if (min) {
		*min = (uint8_t)((bcd_m >> 4) * 10 + (bcd_m & 0x0F));
	}
	if (sec) {
		*sec = (uint8_t)((bcd_s >> 4) * 10 + (bcd_s & 0x0F));
	}
}
