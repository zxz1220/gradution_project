#ifndef __Z_SCREEN_H
#define __Z_SCREEN_H

#include "stm32f10x.h"
#include "Z_common_headfile.h"
/* 在 Z_screen.h 中添加 */
#define ILI9341_SetPoint(x, y, color)    ILI9341_SetPointPixel(x, y)

// 基础布局宏 (供 Z_MAX.c 使用)
#define COLUMN_WAVE_X     100   
#define WAVE_WIDTH        210   
#define ECG_WAVE_TOP      5
#define ECG_WAVE_BOTTOM   55
#define SPO2_WAVE_TOP     65
#define SPO2_WAVE_BOTTOM  115

//首页
#define TITLE_HEIGHT      40    // 顶部标题栏高度
#define GRID_LINE_X       160   // 垂直分割线 (320 / 2)
#define GRID_LINE_Y       140   // 水平分割线 (标题 40 + 剩下的 200 / 2)

// 模式枚举
typedef enum {
    MODE_MENU = 0,    // 主菜单
    MODE_DASHBOARD,   // 综合仪表
    MODE_WAVE_SELECT,    // 实时波形选择
    MODE_ECG_LIVE,      /* 纯心电图全屏绘制 */
    MODE_SPO2_LIVE,     /* 纯血氧图全屏绘制 */
    MODE_HISTORY,     // 历史回顾
    MODE_SETTINGS     // 系统设置
} System_Mode_t;
extern System_Mode_t g_current_mode;


/* 综合仪表盘 */
#define DASH_VAL_X        10    // 数值起始X
#define DASH_WAVE_X       140   // 波形起始X
#define DASH_WAVE_WIDTH   170   // 波形宽度
typedef struct {
    uint16_t hr;
    uint16_t spo2;
    float temp;
    uint8_t bp_high;
    uint8_t bp_low;
    // 用于趋势绘图的微型缓冲区
    uint16_t wave_ptr;     // 绘图水平位置指针
    uint16_t last_y[5];    // 记录心率、血氧、体温、血压的上一个 Y 坐标点   
} HealthData_t;
extern HealthData_t g_HealthData;


typedef struct {
    uint8_t hr_max, hr_min;     /* 心率 (bpm) */
    uint8_t spo2_min;           /* 血氧 (%) */
    float   temp_max, temp_min; /* 体温 (Celsius) */
    uint8_t sbp_max, sbp_min;   /* 收缩压/高压 (mmHg) */
    uint8_t dbp_max, dbp_min;   /* 舒张压/低压 (mmHg) */
} ThresholdConfig;
extern ThresholdConfig g_sys_limit;

typedef enum {
    HIST_HR = 0,    // C列：心率
    HIST_SPO2,      // D列：血氧
    HIST_TEMP,      // E列：体温
    HIST_BP         // F列：血压 (高压/低压)
} History_Type_t;

extern History_Type_t g_hist_view; // 当前查看的分类

void UI_Touch_Handler(void); //触控逻辑封装函数
void UI_Display_Handler(void); //显示逻辑封装函数

void Main_Menu_Display(void);
void Draw_Icon_Dashboard(uint16_t x, uint16_t y);
void Draw_Icon_Waveform(uint16_t x, uint16_t y);
void Draw_Icon_History(uint16_t x, uint16_t y);
void Draw_Icon_Settings(uint16_t x, uint16_t y);

void Dashboard_Page_Init(void);
void Dashboard_Page_Update(void);
void Waveform_Menu_Init(void);
void SPO2_Page_Init(void);
void SPO2_Page_Update(void);
void ECG_Page_Init(void);
void ECG_Page_Update(void);
void Settings_Page_Init(void);
void Settings_Page_Update(void); 
void Dashboard_Alarm_Implementation(void);

void History_Page_Init(void);
void History_Page_Update(void);


#endif
