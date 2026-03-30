#include "Z_common_headfile.h"
#include <math.h>

/* --- 修正后的 NEUQ 矢量线段 (针对 LCD 坐标系) --- */
/* 格式：{x1, y1, x2, y2} */
static const int8_t NEUQ_Lines[][4] = {
    /* N - 修正对角线方向 */ 
    {-35, -15, -35, 15},  /* 左竖 */
    {-35, -15, -15, 15},  /* 对角线 (从左上到右下) */
    {-15, -15, -15, 15},  /* 右竖 */
    
    /* E */ 
    {-5, -15, -5, 15},    /* 背柱 */
    {-5, -15, 10, -15},   /* 上横 */
    {-5, 0, 5, 0},        /* 中横 */
    {-5, 15, 10, 15},     /* 下横 */
    
    /* U */ 
    {15, -15, 15, 15},    /* 左竖 */
    {15, 15, 30, 15},     /* 底横 */
    {30, 15, 30, -15},    /* 右竖 */
    
    /* Q */ 
    {35, -15, 55, -15},   /* 上 */
    {55, -15, 55, 10},    /* 右 */
    {55, 10, 35, 10},     /* 下 */
    {35, 10, 35, -15},    /* 左 */
    {45, 5, 55, 15}       /* 尾巴 */
};

#define TOTAL_LINES 15

/**
 * @brief 渲染矢量 NEUQ
 * @param thick 1为加粗
 */
static void Render_NEUQ_Vector(uint16_t cx, uint16_t cy, float angle, uint16_t color, int thick) {
    int i;
    float rad = angle * 3.1415926f / 180.0f;
    float cos_a = cos(rad);
    float sin_a = sin(rad);

    LCD_SetTextColor(color);

    for (i = 0; i < TOTAL_LINES; i++) {
        /* 计算旋转坐标 */
        int x1 = cx + (int)(NEUQ_Lines[i][0] * cos_a - NEUQ_Lines[i][1] * sin_a);
        int y1 = cy + (int)(NEUQ_Lines[i][0] * sin_a + NEUQ_Lines[i][1] * cos_a);
        int x2 = cx + (int)(NEUQ_Lines[i][2] * cos_a - NEUQ_Lines[i][3] * sin_a);
        int y2 = cy + (int)(NEUQ_Lines[i][2] * sin_a + NEUQ_Lines[i][3] * cos_a);

        ILI9341_DrawLine(x1, y1, x2, y2);

        /* 加粗逻辑：通过微偏移重绘 */
        if (thick) {
            ILI9341_DrawLine(x1 + 1, y1, x2 + 1, y2);
            ILI9341_DrawLine(x1, y1 + 1, x2, y2 + 1);
            ILI9341_DrawLine(x1 + 1, y1 + 1, x2 + 1, y2 + 1);
        }
    }
}

/**
 * @brief 启动 Logo 动画
 */
void Show_Startup_Logo(void) {
    int frame;
    float angle = 0.0f;
    uint16_t cx = 160; 
    uint16_t cy = 90;

    /* 1. 强制黑底 */
    LCD_SetBackColor(BLACK);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    
    /* 2. 绘制静态 LOADING (白字) */
    LCD_SetColors(WHITE, BLACK);
    LCD_SetFont(&Font16x24);
    ILI9341_DispString_EN(100, 160, "LOADING..");

    /* 3. 旋转过程 (白色线段) */
    for (frame = 0; frame < 100; frame++) {
        /* 擦除旧帧 (用背景色黑色画一遍) */
        Render_NEUQ_Vector(cx, cy, angle, BLACK, 0);

        angle += 10.0f; 
        if (angle >= 360.0f) angle -= 360.0f;

        /* 绘制新帧 (白色) */
        Render_NEUQ_Vector(cx, cy, angle, WHITE, 0);

        /* LOADING 点动画 */
        if(frame % 20 == 0) ILI9341_DispString_EN(100, 160, "LOADING.. ");
        if(frame % 20 == 10) ILI9341_DispString_EN(100, 160, "LOADING...");

        delay_ms(30);
    }

    /* 4. 最终状态：停止旋转，变为蓝色并加粗 */
    /* 擦除最后一帧白色线 */
    Render_NEUQ_Vector(cx, cy, angle, BLACK, 0);
    
    /* 以蓝色加粗重新绘制 (角度重置为0，即正向显示) */
    Render_NEUQ_Vector(cx, cy, 0, BLUE, 1); 

    /* 5. 文字变更为 READY */
    LCD_SetTextColor(CYAN);
    ILI9341_DispString_EN(85, 160, "SYSTEM READY");
    delay_ms(1500);

    /* 6. 最终清屏进入主界面 */
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
}