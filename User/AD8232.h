#ifndef __AD8232_H
#define	__AD8232_H

#include "stm32f10x.h"
#include "bsp_adc.h"
#include "bsp_GENERALTim.h"
#include <string.h> // 增加memset依赖

// 硬件参数（28.5周期+双通道+12MHz ADC时钟）
#define ADC_MAX_VALUE    4095      // ADC满量程值
#define ADC_REF_VOLTAGE  3300.0f   // ADC参考电压(mV)
#define FILTER_SIZE      16        // 滑动平均窗口大小
#define SAMPLING_RATE    210.5f    // 双通道实际采样率210.5Hz



float AD8232_Filter(void);


#endif 

