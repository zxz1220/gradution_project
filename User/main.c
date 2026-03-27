#include "stm32f10x.h"
#include "./usart/bsp_usart.h"	
#include "max30102.h"
#include "delay.h"
#include <stdio.h>
#include "Z_MLX.h"
#include "Z_JFC.h"
#include "bsp_adc.h"
#include "AD8232.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./font/fonts.h" // 确保包含字体定义
#include "Z_screen.h"
int main(void)
{
    char disp_buf[32];
    // 1. 基础硬件初始化
    delay_init(72);      //
    USART_Config();     //
    JFC103_USART_Config(); // 新增：JFC103 串口初始化
    Monitor_Layout_Init();
    // 2. 调用封装后的 MAX30102 初始化 (含5秒校准)
    // MAX30102_App_Init(); 
    // 3. 调用封装后的 MLX90614 初始化
    // MLX90614_Init();
    // delay_ms(50);
    JFC103_SendCmd(0x8A);
    // ADCx_Init();
    while(1)
    {
    /* 2. JFC 数值更新 (左侧区域) */
        if(JFC_DataReady) 
        {
            // 获取结构体指针
            JFC103_Data_t *health = (JFC103_Data_t *)JFC_RawBuf;
            uint8_t i;

            /* --- 1. 更新左侧静态数值 --- */
            LCD_SetFont(&Font8x16);
            
            // 血氧饱和度
            LCD_SetColors(GREEN, BLACK);
            sprintf(disp_buf, "%d%%", health->spo2);
            ILI9341_DispString_EN(COLUMN_VAL_X+55, 80, disp_buf);

            // 血压
            // LCD_SetColors(CYAN, BLACK);
            // sprintf(disp_buf, "%d/%d", health->systolic, health->diastolic);
            // ILI9341_DispString_EN(COLUMN_VAL_X, 195, disp_buf);

            /* --- 2. 逐点绘制 64 个波形数据 --- */
            for(i = 0; i < 64; i++) {
                DualWave_Update_JFC(health->acdata[i], health->spo2);
                // 根据 JFC 模块输出频率调整延时
                // 假设 1 秒发一包，延时设为 15ms 左右波形最平滑
                delay_ms(5); 
            }

            JFC_DataReady = 0; // 重置数据标志
        }
    }
				
}

