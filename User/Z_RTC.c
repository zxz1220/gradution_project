#include "Z_RTC.h"

/**
 * @brief RTC 纯净初始化
 * @note 强制上电归零，作为相对开机时间戳
 */
void RTC_Init(void) {
    /* 1. 时钟使能 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    /* 2. 强制开启 LSE */
    RCC_LSEConfig(RCC_LSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForSynchro();

    /* 3. 配置 1Hz 频率 (1秒走一次) */
    RTC_SetPrescaler(32767);
    RTC_WaitForLastTask();

    /* 4. 关键：彻底抛弃日期，上电即为 0 秒 */
    RTC_SetCounter(0); 
    RTC_WaitForLastTask();
}