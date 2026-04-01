#ifndef _z_common_headfile_h_
#define _z_common_headfile_h_

#include "./usart/bsp_usart.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./lcd/bsp_xpt2046_lcd.h"	
#include "./sdio/bsp_sdio_sdcard.h"
#include "./Systick/bsp_SysTick.h"

#include "./FATFS/ff.h"
#include "./FATFS/diskio.h"
#include "./FATFS/ffconf.h"
#include "./FATFS/integer.h"

#include "./font/fonts.h" // 确保包含字体定义
#include "max30102.h"
#include "delay.h"
#include "bsp_adc.h"
#include "AD8232.h"
#include "Z_screen.h"
#include "Z_MAX.h"
#include "Z_MLX.h"
#include "Z_JFC.h"
#include "Z_logo.h"
#include "Z_TF.h"
#include "Z_RTC.h"

#endif
