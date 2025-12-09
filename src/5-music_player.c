/**
 * @author zjm54321
 * @brief 音乐播放器
 * @attention 需要将频率设置为 27MHz
 */
#include <utility.h>
#include <stc51.h>
#include <stdint.h>
#include <stdbool.h>
#include <compiler.h>

#define BUZZER P06

// 节拍定义 (基准 BPM = 92)
// Quarter (四分音符) = 60000 / 92 ≈ 652 ms
// 八分之一 个三十二分音符 = 652 / 8 / 8 ≈ 10 ms
inline void delay_eighth_of_thirtysecond_note(void) {
    unsigned char data i, j, k;

    NOP();
    NOP();
    i = 2;
    j = 7;
    k = 159;
    do {
        do {
            while (--k)
                ;
        } while (--j);
    } while (--i);
}

typedef enum Tone {
    ToneLow1,
    ToneLow2,
    ToneLow3,
    ToneLow4,
    ToneLow5,
    ToneLow6,
    ToneLow7,
    ToneMid1,
    ToneMid2,
    ToneMid3,
    ToneMid4,
    ToneMid5,
    ToneMid6,
    ToneMid7,
    ToneHigh1,
    ToneHigh2,
    ToneHigh3,
    ToneHigh4,
    ToneHigh5,
    ToneHigh6,
    ToneHigh7,
    ToneRest = 255
} Tone_t;

typedef enum Duration {
    ThirtySecondNote = 1,
    SixteenthNote = 2,
    EighthNote = 4,
    QuarterNote = 8,
    HalfNote = 16,
    WholeNote = 32,
    StopNote = 0,
} Duration_t;

typedef struct Note {
    Tone_t tone;
    Duration_t duration;
} Note_t;

typedef enum Volume {
    VolOff,
    Vol1,
    Vol2,
    Vol3,
    Vol4,
    Vol5,
    Vol6,
    Vol7,
    Vol8,
    Vol9,
} Volume_t;

typedef struct Timerdata {
    uint16_t reload_high;
    uint16_t reload_low;
    bool high_phase;
} Timerdata_t;

// 设置音乐曲谱
const Note_t code Music[] = {
    // --- 弱起小节 (Pickup) ---
    // 3 2 (中音)
    {ToneMid3, QuarterNote},
    {ToneMid2, QuarterNote},

    // --- 第1小节 ---
    {ToneMid1, QuarterNote},
    {ToneMid5, QuarterNote},
    {ToneMid6, EighthNote},
    {ToneMid5, EighthNote},
    {ToneMid3, SixteenthNote},
    {ToneMid2, SixteenthNote},
    {ToneMid1, EighthNote},

    // --- 第2小节 ---
    {ToneMid2, EighthNote},
    {ToneMid3, SixteenthNote},
    {ToneMid2, SixteenthNote},
    {ToneMid1, EighthNote},
    {ToneMid2, EighthNote},
    {ToneMid3, QuarterNote},
    {ToneMid1, EighthNote},
    {ToneLow7, EighthNote},

    // --- 第3小节 ---
    {ToneMid1, QuarterNote},
    {ToneMid3, QuarterNote},
    {ToneMid4, EighthNote},
    {ToneMid3, EighthNote},
    {ToneMid2, EighthNote},
    {ToneMid1, EighthNote},

    // --- 第4小节 ---
    {ToneMid2, EighthNote},
    {ToneMid3, SixteenthNote},
    {ToneMid2, SixteenthNote},
    {ToneMid1, EighthNote},
    {ToneLow7, EighthNote},
    {ToneMid1, QuarterNote},
    {ToneMid3, EighthNote},
    {ToneMid2, EighthNote},

    // --- 第5小节 ---
    {ToneMid1, QuarterNote},
    {ToneMid5, QuarterNote},
    {ToneMid6, EighthNote},
    {ToneMid5, EighthNote},
    {ToneMid3, SixteenthNote},
    {ToneMid2, SixteenthNote},
    {ToneMid1, EighthNote},

    // --- 第6小节 ---
    {ToneMid2, EighthNote},
    {ToneMid3, SixteenthNote},
    {ToneMid2, SixteenthNote},
    {ToneMid1, EighthNote},
    {ToneMid2, EighthNote},
    {ToneMid3, QuarterNote},
    {ToneLow6, EighthNote},
    {ToneLow7, EighthNote},

    // --- 第7小节 ---
    {ToneMid1, QuarterNote},
    {ToneMid3, QuarterNote},
    {ToneMid4, EighthNote},
    {ToneMid3, EighthNote},
    {ToneMid2, EighthNote},
    {ToneMid1, EighthNote},

    // --- 第8小节 ---
    {ToneMid2, EighthNote},
    {ToneMid3, SixteenthNote},
    {ToneMid2, SixteenthNote},
    {ToneMid1, EighthNote},
    {ToneLow7, EighthNote},
    {ToneMid1, HalfNote},

    {ToneRest, StopNote} // 结束标记
};

void timer0_init(void);
void set_buzzer(Tone_t tone, Volume_t vol);
void play_note(Note_t note);
void update_led(Tone_t tone);

static Timerdata_t timerdata = {0, 0, false};

int32_t main(void) {
    timer0_init();
    EA = 1; // 全局中断使能

    for (;;) {
        while (P33)
            ;

        uint64_t i = 0;
        // 播放整首曲子
        while (Music[i].duration != StopNote) {
            play_note(Music[i]);
            i++;
        }
    }
}

void timer0_init(void) {
    AUXR &= 0x7F; // 定时器时钟12T模式
    TMOD &= 0xF0; // 设置定时器模式
    TMOD |= 0x01; // 设置定时器模式
    TF0 = 0;      // 清除TF0标志
    ET0 = 1;      // 使能定时器0中断
}

void timer0_isr(void) interrupt(1) {
    if (timerdata.high_phase) {
        TL0 = (uint8_t)timerdata.reload_low;
        TH0 = (uint8_t)(timerdata.reload_low >> 8);
        timerdata.high_phase = 0;
    } else {
        TL0 = (uint8_t)timerdata.reload_high;
        TH0 = (uint8_t)(timerdata.reload_high >> 8);
        timerdata.high_phase = 1;
    }

    P2 = (P2 & 0x1F) | 0xA0;
    BUZZER = timerdata.high_phase;
    P2 = ((P2 & 0x1f) | 0x80);
}

/// [21个音符] x [10个音量等级] x [2个重装载值(High_Reload,Low_Reload)]
/// 频率基准：STC15 @ 27MHz, Timer 12T Mode (计数频率 2.25MHz)
/// 音量曲线：指数型 (Level 1=0.5%占空比 ... Level 10=50%占空比)
const uint16_t code TONE_TABLE[21][10][2] = {
    // === 低音区 (Low 1-7) ===

    // Index 0: Low 1 (466Hz, T=4828)
    {{0xFFC7, 0xED5C},
     {0xFF8F, 0xED94},
     {0xFF42, 0xEDE1},
     {0xFEC1, 0xEE62},
     {0xFE09, 0xEF1A},
     {0xFCF8, 0xF02B},
     {0xFB49, 0xF1DA},
     {0xF963, 0xF3C0},
     {0xF57B, 0xF7A8},
     {0xF692, 0xF692}},
    // Index 1: Low 2 (523Hz, T=4302)
    {{0xFFCD, 0xEF57},
     {0xFF9B, 0xEF89},
     {0xFF56, 0xEFCE},
     {0xFEE3, 0xF041},
     {0xFE40, 0xF0E4},
     {0xFD4D, 0xF1D7},
     {0xFBCC, 0xF358},
     {0xFBA0, 0xF504},
     {0xF69D, 0xF887},
     {0xF799, 0xF799}},
    // Index 2: Low 3 (587Hz, T=3833)
    {{0xFFD3, 0xF124},
     {0xFFA6, 0xF151},
     {0xFF69, 0xF18E},
     {0xFF03, 0xF1F4},
     {0xFE71, 0xF286},
     {0xFD99, 0xF35E},
     {0xFC40, 0xF4B7},
     {0xFD02, 0xF635},
     {0xF79D, 0xF95A},
     {0xF883, 0xF883}},
    // Index 3: Low 4 (659Hz, T=3414)
    {{0xFFD7, 0xF2C4},
     {0xFFB0, 0xF2EB},
     {0xFF79, 0xF322},
     {0xFF1E, 0xF37D},
     {0xFE9D, 0xF3FE},
     {0xFDDD, 0xF4BE},
     {0xFCA9, 0xF5F2},
     {0xFD8E, 0xF74D},
     {0xF882, 0xFA19},
     {0xF955, 0xF955}},
    // Index 4: Low 5 (698Hz, T=3223)
    {{0xFFDA, 0xF38F},
     {0xFFB4, 0xF3B5},
     {0xFF81, 0xF3E8},
     {0xFF2A, 0xF43F},
     {0xFEB0, 0xF4B9},
     {0xFDF7, 0xF572},
     {0xFCD4, 0xF695},
     {0xFDDA, 0xF7CF},
     {0xF8ED, 0xFA76},
     {0xF9B4, 0xF9B4}},
    // Index 5: Low 6 (784Hz, T=2869)
    {{0xFFDF, 0xF4E8},
     {0xFFBD, 0xF50A},
     {0xFF8F, 0xF538},
     {0xFF42, 0xF585},
     {0xFED3, 0xF5F4},
     {0xFE2E, 0xF699},
     {0xFD2D, 0xF79A},
     {0xFE66, 0xF8B1},
     {0xF9B4, 0xFB23},
     {0xFA65, 0xFA65}},
    // Index 6: Low 7 (880Hz, T=2556)
    {{0xFFE4, 0xF613},
     {0xFFC4, 0xF633},
     {0xFF9B, 0xF65C},
     {0xFF56, 0xF6A1},
     {0xFEF4, 0xF703},
     {0xFE61, 0xF796},
     {0xFD7C, 0xF87B},
     {0xFECE, 0xF979},
     {0xFA66, 0xFBAB},
     {0xFB02, 0xFB02}},

    // === 中音区 (Mid 1-7) ===

    // Index 7: Mid 1 (932Hz, T=2414)
    {{0xFFE6, 0xF69D},
     {0xFFC7, 0xF6BC},
     {0xFFA1, 0xF6E2},
     {0xFF61, 0xF722},
     {0xFF05, 0xF77E},
     {0xFE78, 0xF80B},
     {0xFD9F, 0xF8E4},
     {0xFF07, 0xF9DD},
     {0xFAB5, 0xFBEB},
     {0xFB49, 0xFB49}},
    // Index 8: Mid 2 (1047Hz, T=2149)
    {{0xFFE8, 0xF7AA},
     {0xFFCD, 0xF7C5},
     {0xFFAB, 0xF7E7},
     {0xFF72, 0xF820},
     {0xFF21, 0xF871},
     {0xFEA4, 0xF8EE},
     {0xFDE1, 0xF9AF},
     {0xFF61, 0xFA8F},
     {0xFB4A, 0xFC5D},
     {0xFBCD, 0xFBCD}},
    // Index 9: Mid 3 (1175Hz, T=1914)
    {{0xFFEB, 0xF88E},
     {0xFFD3, 0xF8A6},
     {0xFFB5, 0xF8C4},
     {0xFF82, 0xF8F7},
     {0xFF3A, 0xF93F},
     {0xFECB, 0xF9AE},
     {0xFE1C, 0xFA5D},
     {0xFFAC, 0xFB33},
     {0xFBD2, 0xFCCC},
     {0xFC43, 0xFC43}},
    // Index 10: Mid 4 (1319Hz, T=1705)
    {{0xFFED, 0xF95E},
     {0xFFD8, 0xF973},
     {0xFFBC, 0xF98F},
     {0xFF8F, 0xF9BC},
     {0xFF4E, 0xF9FD},
     {0xFEED, 0xFA62},
     {0xFE51, 0xFAFE},
     {0xFFEE, 0xFBBD},
     {0xFC44, 0xFD23},
     {0xFCAB, 0xFCAB}},
    // Index 11: Mid 5 (1397Hz, T=1610)
    {{0xFFEE, 0xF9BD},
     {0xFFDA, 0xF9D1},
     {0xFFC0, 0xF9EB},
     {0xFF96, 0xFA15},
     {0xFF58, 0xFA52},
     {0xFEFA, 0xFAB0},
     {0xFE66, 0xFB44},
     {0x000C, 0xFBFC},
     {0xFC78, 0xFD4A},
     {0xFCDB, 0xFCDB}},
    // Index 12: Mid 6 (1568Hz, T=1434)
    {{0xFFEF, 0xFA6C},
     {0xFFDE, 0xFA7D},
     {0xFFC7, 0xFA94},
     {0xFFA2, 0xFAB8},
     {0xFF69, 0xFAF1},
     {0xFF15, 0xFB44},
     {0xFE90, 0xFBC6},
     {0x0045, 0xFC69},
     {0xFCDA, 0xFD96},
     {0xFD33, 0xFD33}},
    // Index 13: Mid 7 (1760Hz, T=1278)
    {{0xFFF1, 0xFB08},
     {0xFFE2, 0xFB17},
     {0xFFCD, 0xFB2C},
     {0xFFAC, 0xFB4D},
     {0xFF79, 0xFB80},
     {0xFF2E, 0xFBC8},
     {0xFEC2, 0xFC3E},
     {0x0084, 0xFCCE},
     {0xFD31, 0xFDDE},
     {0xFD81, 0xFD81}},

    // === 高音区 (High 1-7) ===

    // Index 14: High 1 (1865Hz, T=1206)
    {{0xFFF1, 0xFB4F},
     {0xFFE3, 0xFB5D},
     {0xFFD0, 0xFB70},
     {0xFFB1, 0xFB8F},
     {0xFF82, 0xFBBF},
     {0xFF3A, 0xFC02},
     {0xFED9, 0xFC71},
     {0x00AD, 0xFD05},
     {0xFD59, 0xFE03},
     {0xFDA5, 0xFDA5}},
    // Index 15: High 2 (2094Hz, T=1074)
    {{0xFFF3, 0xFBD2},
     {0xFFE6, 0xFBDF},
     {0xFFD5, 0xFBF0},
     {0xFFB9, 0xFC0C},
     {0xFF8F, 0xFC36},
     {0xFF4F, 0xFC71},
     {0xFEF9, 0xFCD5},
     {0x00F8, 0xFD58},
     {0xFDA2, 0xFE3A},
     {0xFDE7, 0xFDE7}},
    // Index 16: High 3 (2351Hz, T=957)
    {{0xFFF4, 0xFC47},
     {0xFFE9, 0xFC52},
     {0xFFDA, 0xFC61},
     {0xFFC1, 0xFC7A},
     {0xFF9C, 0xFC9F},
     {0xFF63, 0xFCD4},
     {0xFF16, 0xFD2D},
     {0x013B, 0xFDA1},
     {0xFDE3, 0xFE6B},
     {0xFE21, 0xFE21}},
    // Index 17: High 4 (2633Hz, T=854)
    {{0xFFF5, 0xFCAD},
     {0xFFEC, 0xFCB6},
     {0xFFDE, 0xFCC4},
     {0xFFC8, 0xFCDA},
     {0xFFA7, 0xFCFB},
     {0xFF74, 0xFD2A},
     {0xFF30, 0xFD7A},
     {0x0176, 0xFDE1},
     {0xFE1D, 0xFE95},
     {0xFE55, 0xFE55}},
    // Index 18: High 5 (2792Hz, T=805)
    {{0xFFF6, 0xFCDE},
     {0xFFED, 0xFCE8},
     {0xFFDF, 0xFCF6},
     {0xFFCB, 0xFD0A},
     {0xFFAC, 0xFD29},
     {0xFF7C, 0xFD59},
     {0xFF3C, 0xFD99},
     {0x0192, 0xFE03},
     {0xFE38, 0xFEAD},
     {0xFE6D, 0xFE6D}},
    // Index 19: High 6 (3134Hz, T=717)
    {{0xFFF7, 0xFD35},
     {0xFFEF, 0xFD3D},
     {0xFFE3, 0xFD49},
     {0xFFD1, 0xFD5B},
     {0xFFB5, 0xFD77},
     {0xFF8B, 0xFDDA},
     {0xFF52, 0xFDDD},
     {0x01C4, 0xFE3B},
     {0xFE6A, 0xFEE7},
     {0xFE99, 0xFE99}},
    // Index 20: High 7 (3517Hz, T=639)
    {{0xFFF8, 0xFD82},
     {0xFFF0, 0xFD8A},
     {0xFFE6, 0xFD94},
     {0xFFD6, 0xFDA4},
     {0xFFBD, 0xFDBD},
     {0xFF97, 0xFDE3},
     {0xFF63, 0xFE16},
     {0x01F0, 0xFE6F},
     {0xFE96, 0xFF05},
     {0xFEC0, 0xFEC0}}};

inline void set_buzzer(Tone_t tone, Volume_t vol) {
    if (vol == VolOff || tone == ToneRest) {
        TR0 = 0;
        BUZZER = 0;
        return;
    }

    TR0 = 0;
    timerdata.reload_high = TONE_TABLE[tone][vol][0];
    timerdata.reload_low = TONE_TABLE[tone][vol][1];
    timerdata.high_phase = true;

    TL0 = (uint8_t)timerdata.reload_high;
    TH0 = (uint8_t)(timerdata.reload_high >> 8);

    BUZZER = 1;
    TR0 = 1;
}

// 播放单个音符
inline void play_note(Note_t note) {

    // 休止符处理
    if (note.tone == ToneRest) {
        set_buzzer(ToneRest, 0);
        for (uint16_t i = 0; i < note.duration * 8; i++)
            delay_eighth_of_thirtysecond_note();
        return;
    }

    // --- 情感处理：模仿歌唱的“渐强-保持-渐弱” ---

    // 1. Attack (起音): 快速从弱到强 (一个三十二分音符)
    for (Volume_t v = Vol1; v < Vol9; v++) {
        set_buzzer(note.tone, v);
        delay_eighth_of_thirtysecond_note();
    }

    // 2. Sustain (保持):
    set_buzzer(note.tone, Vol9);
    for (uint8_t i = 0; i < (note.duration - ThirtySecondNote) * 8; i++)
        delay_eighth_of_thirtysecond_note();

    // 3. Release (释音): 快速衰减 (30ms)
    for (Volume_t v = Vol8; v > VolOff; v--) {
        set_buzzer(note.tone, v);
        delay_eighth_of_thirtysecond_note();
    }

    set_buzzer(ToneRest, 0);
    delay_eighth_of_thirtysecond_note();
}
