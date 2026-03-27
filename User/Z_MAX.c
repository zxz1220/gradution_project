#include "Z_MAX.h"
#include "./usart/bsp_usart.h"	
#include "delay.h"

/* --- 静态私有缓冲区 --- */
static uint32_t aun_ir_buffer[BUFFER_SIZE];  //
static uint32_t aun_red_buffer[BUFFER_SIZE]; //
static int32_t  n_ir_buffer_length = BUFFER_SIZE;

/* --- 外部可访问的测量结果 --- */
int32_t n_heart_rate; 
int32_t n_sp02;
int8_t  ch_hr_valid;
int8_t  ch_spo2_valid;

/**
  * @brief  初始化应用层：包括硬件初始化和前500个样本采集
  */
void MAX30102_App_Init(void)
{
    int i;
    
    // 1. 调用底层硬件初始化
    MAX30102_Init(); 
    
    printf("\r\n--- 传感器初始化中 (5秒)... ---\r\n");

    // 2. 采集初始500个样本
    for(i = 0; i < n_ir_buffer_length; i++)
    {
        while(MAX30102_INT == 1); // 等待硬件中断引脚变低
        
        // 直接使用底层读取函数
        maxim_max30102_read_fifo(&aun_red_buffer[i], &aun_ir_buffer[i]);
        
        if(i % 100 == 0) printf("."); 
    }
    
    // 3. 计算初始心率和血氧
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, 
                                          &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
    
    printf("\r\n--- 初始化完成 ---\r\n");
}

/**
  * @brief  应用层循环：处理滑动窗口和算法更新
  */
void MAX30102_App_Update(void)
{
    int i;
    
    // A. 舍去前100组，平移后400组
    for(i = 100; i < 500; i++)
    {
        aun_red_buffer[i-100] = aun_red_buffer[i];
        aun_ir_buffer[i-100] = aun_ir_buffer[i];
    }
    
    // B. 采集新的100组样本
    for(i = 400; i < 500; i++)
    {
        while(MAX30102_INT == 1); // 严格同步硬件中断引脚
        maxim_max30102_read_fifo(&aun_red_buffer[i], &aun_ir_buffer[i]); //
    }

    // C. 运行核心算法刷新结果
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, 
                                          &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
    
    // D. 串口结果打印
    if(ch_hr_valid == 1 && ch_spo2_valid == 1)
    {
        printf("\r\n[DATA] 心率: %d BPM | 血氧: %d%%", n_heart_rate, n_sp02);
    }
    else
    {
        printf("\r\n[WAIT] 正在寻找稳定信号...");
    }
}