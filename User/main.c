#include "stm32f10x.h"
#include "./usart/bsp_usart.h"	
#include "max30102.h"
#include "delay.h"
#include <stdio.h>
#include "Z_MLX.h"
#include "Z_JFC.h"
#include "bsp_adc.h"
#include "AD8232.h"
int main(void)
{
    float ecg_val = 0;
    // 1. 基础硬件初始化
    delay_init(72);      //
    USART_Config();     //
    JFC103_USART_Config(); // 新增：JFC103 串口初始化
    // 2. 调用封装后的 MAX30102 初始化 (含5秒校准)
    // MAX30102_App_Init(); 
    // 3. 调用封装后的 MLX90614 初始化
    // MLX90614_Init();
    // delay_ms(50);
    // JFC103_SendCmd(0x8A);
    // ADCx_Init();
    while(1)
    {
        // if(JFC_DataReady)
        //         {
        //             JFC103_Data_t *health = (JFC103_Data_t *)JFC_RawBuf;
                    
        //             printf("--- Health Report ---\r\n");
        //             printf("HeartRate: %d bpm | Spo2: %d%%\r\n", health->heartrate, health->spo2);
        //             printf("BloodPressure: %d/%d mmHg\r\n", health->systolic, health->diastolic);
                    
        //             JFC_DataReady = 0; // 处理完清除标志
        //         }
		// ecg_val = AD8232_Filter();
        // printf("%.2f\n", ecg_val);
        // delay_ms(5);	
    }
				
}

