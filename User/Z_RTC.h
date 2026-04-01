#ifndef __Z_RTC_H
#define __Z_RTC_H

#include "stm32f10x.h"

/* 时间结构体定义 */
typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
} RTC_TimeTypeDef;

extern RTC_TimeTypeDef g_NowTime;

/* 函数声明 */
void RTC_Init(void);


#endif

