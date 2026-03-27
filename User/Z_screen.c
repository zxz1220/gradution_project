#include "Z_screen.h"
static uint16_t s_x_ptr = COLUMN_WAVE_X;
static int16_t  last_ecg_y = 30; // 第一层（心电/acdata）
static int16_t  last_spo2_y = 90; // 第二层（血氧模拟）

void Monitor_Layout_Init(void) {
    ILI9341_Init();
    ILI9341_GramScan(3); 
    
    LCD_SetColors(WHITE, BLACK);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);

    // 绘制装饰线
    LCD_SetTextColor(GREY);
    ILI9341_DrawLine(0, 60, 320, 60);    
    ILI9341_DrawLine(0, 120, 320, 120);  
    ILI9341_DrawLine(COLUMN_WAVE_X - 5, 0, COLUMN_WAVE_X - 5, 240); 

    LCD_SetFont(&Font8x16);
    // 第一层：心电标签（黄色）
    LCD_SetTextColor(YELLOW); ILI9341_DispString_CH(COLUMN_VAL_X, 20, "心电");
    // 第二层：血氧标签（绿色）
    LCD_SetTextColor(GREEN);  ILI9341_DispString_CH(COLUMN_VAL_X, 80, "血氧：");
    // 第三层：血压标签
    LCD_SetTextColor(CYAN);   ILI9341_DispString_CH(COLUMN_VAL_X, 170, "血压");
}

/**
 * @brief 同步更新双层波形
 * @param ac_val: 来自 health->acdata[i] (心律数据)
 * @param spo2_val: 来自 health->spo2 (血氧数值，用于模拟)
 */
void DualWave_Update_JFC(int8_t ac_val, uint8_t spo2_val) {
    static uint16_t phase = 0;
    int16_t cur_ecg_y, cur_spo2_y;
    uint16_t j, clean_x;
    int8_t mock_offset;

    // 1. 第一层：心电波形映射 (使用 acdata: -128~127 -> 5~55)
    cur_ecg_y = 30 - (ac_val * 50 / 256);

    // 2. 第二层：血氧模拟波形映射 (基于 spo2 数值制作起伏)
    if (phase < 5)       mock_offset = phase * 2;
    else if (phase < 15) mock_offset = 10 - (phase-5);
    else                 mock_offset = 0-
    cur_spo2_y = 115 - (spo2_val * 45 / 100) - mock_offset;

    // 3. 刷子逻辑：清除前方 5 像素
    for(j = 1; j <= 5; j++) {
        clean_x = s_x_ptr + j;
        if (clean_x >= (COLUMN_WAVE_X + WAVE_WIDTH)) 
            clean_x = COLUMN_WAVE_X + (clean_x - (COLUMN_WAVE_X + WAVE_WIDTH));
        
        LCD_SetTextColor(BLACK);
        ILI9341_DrawLine(clean_x, 5,  clean_x, 55);  
        ILI9341_DrawLine(clean_x, 65, clean_x, 115); 
    }

    // 4. 绘制波形 (注意颜色和层级)
    LCD_SetTextColor(YELLOW); // 第一层：心电(acdata)用黄色
    ILI9341_DrawLine(s_x_ptr - 1, last_ecg_y, s_x_ptr, cur_ecg_y);
    
    LCD_SetTextColor(GREEN);  // 第二层：血氧(模拟)用绿色
    ILI9341_DrawLine(s_x_ptr - 1, last_spo2_y, s_x_ptr, cur_spo2_y);

    // 5. 状态更新
    last_ecg_y = cur_ecg_y;
    last_spo2_y = cur_spo2_y;
    s_x_ptr++;
    phase = (phase + 1) % 30;
    
    if (s_x_ptr >= (COLUMN_WAVE_X + WAVE_WIDTH)) s_x_ptr = COLUMN_WAVE_X;
}