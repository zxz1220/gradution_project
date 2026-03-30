#include "Z_common_headfile.h"

// /* --- 静态私有缓冲区 --- */
// static uint32_t aun_ir_buffer[BUFFER_SIZE];  //
// static uint32_t aun_red_buffer[BUFFER_SIZE]; //
// static int32_t  n_ir_buffer_length = BUFFER_SIZE;

// /* --- 外部可访问的测量结果 --- */
// int32_t n_heart_rate; 
// int32_t n_sp02;
// int8_t  ch_hr_valid;
// int8_t  ch_spo2_valid;
// int8_t is_finger_on = 0;
// /**
//   * @brief  初始化应用层：包括硬件初始化和前500个样本采集
//   */
// void MAX30102_App_Soft_Init(void)
// {
//     int i;
//     MAX30102_Init();
//     // 提示用户
//     LCD_SetColors(RED, BLACK);
//     LCD_SetFont(&Font8x16);
//     ILI9341_DispString_EN(COLUMN_WAVE_X + 10, 85, "SENSING...WAIT 5S");

//     for(i = 0; i < 500; i++)
//     {
//         while(MAX30102_INT == 1); 
//         maxim_max30102_read_fifo(&aun_red_buffer[i], &aun_ir_buffer[i]);
        
//         // 初始化时也绘制波形，让用户知道在运行
//         // MAX30102_RawWave_Update(aun_ir_buffer[i]); 
//     }
    
//     maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, 500, aun_red_buffer, 
//                                           &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
    
//     // 清除提示文字
//     ILI9341_Clear(COLUMN_WAVE_X, 80, WAVE_WIDTH, 30);
// }

// /**
//   * @brief  应用层循环：处理滑动窗口和算法更新
//   */
// void MAX30102_App_Update(void)
// {
//     /* --- C89 变量定义置顶 --- */
//     int i;
//     uint32_t max_ir = 0;
//     uint32_t min_ir = 0xFFFFFFFF;

//     /* A. 数据平移逻辑 (原有代码) */
//     for(i = 100; i < 500; i++) {
//         aun_red_buffer[i-100] = aun_red_buffer[i];
//         aun_ir_buffer[i-100] = aun_ir_buffer[i];
//     }
    
//     /* B. 采集新样本并实时画图 (原有代码) */
//     for(i = 400; i < 500; i++) {
//         while(MAX30102_INT == 1); 
//         maxim_max30102_read_fifo(&aun_red_buffer[i], &aun_ir_buffer[i]);
//         MAX30102_RawWave_Update(aun_ir_buffer[i]); // 调用绘图
        
//         // 实时记录这一组样本的极值
//         if(aun_ir_buffer[i] > max_ir) max_ir = aun_ir_buffer[i];
//         if(aun_ir_buffer[i] < min_ir) min_ir = aun_ir_buffer[i];
//     }

//     /* C. 脱落判定逻辑 */
//     // 如果 100 个样本内波动小于 300 (阈值根据实际调整)，视为无手指
//     if((max_ir - min_ir) < 300) {
//         is_finger_on = 0;
//     } else {
//         is_finger_on = 1;
//     }

//     /* D. 运行算法 (原有代码) */
//     maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, 
//                                           &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
// }