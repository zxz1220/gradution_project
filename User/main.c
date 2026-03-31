#include "stm32f10x.h"
#include <stdio.h>
#include "Z_common_headfile.h"

int main(void)
{
    /*变量声明*/

    /*变量声明*/

    /*外设初始化*/
    delay_init(72);      // 延时初始化
    USART_Config();     // 串口初始化
    ADCx_Init();        //ADC初始化
    JFC103_USART_Config(); //JFC103 串口初始化
    ILI9341_Init();     //屏幕初始化
    ILI9341_GramScan(3); //设置屏幕
    XPT2046_Init();     //触摸初始化
    MLX90614_Init(); //温度模块
    Show_Startup_Logo();    //logo初始化
    Main_Menu_Display();    //主页面显示
    g_current_mode = MODE_MENU; //当前主页面菜单
   /*外设初始化*/
    delay_ms(5);
    while(1)
    {   
        /*触控响应*/
        UI_Touch_Handler();
        /*显示响应*/ 
        UI_Display_Handler();
    }
				
}

