#ifndef __Z_SCREEN_H
#define __Z_SCREEN_H
#include "./lcd/bsp_ili9341_lcd.h"
#include "./lcd/bsp_xpt2046_lcd.h"
#include <stdio.h>
/* 常用颜色定义 */

/* 布局宏定义：左侧数值，右侧波形 */
#define COLUMN_VAL_X      5    
#define COLUMN_WAVE_X     100   
#define WAVE_WIDTH        210   

/* 第一层：心电波形区域 (对应“心电”文字层) */
#define ECG_WAVE_TOP      5
#define ECG_WAVE_BOTTOM   55

/* 第二层：血氧波形区域 (对应“血氧”文字层) */
#define JFC_WAVE_TOP      65
#define JFC_WAVE_BOTTOM   115

/* 颜色定义补全 */
#ifndef GREY
#define GREY    0xF7DE
#define YELLOW  0xFFE0
#endif

/* 函数声明 */
void Monitor_Layout_Init(void);
void DualWave_Update_JFC(int8_t ac_val, uint8_t spo2_val);

#endif // __Z_SCREEN_H