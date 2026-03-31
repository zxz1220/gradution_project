#include "Z_common_headfile.h"

static uint16_t last_hr = 0, last_spo2 = 0;
static uint8_t last_bp_h = 0, last_bp_l = 0;
static float last_temp = 0;

static uint8_t last_spo2_val = 0xFF; // 初始化为0xFF，确保第一次进入必刷新
static uint16_t last_y_pos = 120;    // 记录上一帧Y坐标

System_Mode_t g_current_mode = MODE_MENU;
strType_XPT2046_Coordinate touch_pos;
HealthData_t g_HealthData;


/* 触控交互与页面跳转逻辑封装 - 优化版 */
void UI_Touch_Handler(void) 
{
    /* 1. C89 变量定义置顶 */
    strType_XPT2046_Coordinate touch_pos;
    static uint8_t is_touch_locked = 0; // 静态变量：触控锁定标志位

    /* 2. 检测是否有触控按下 */
    if (XPT2046_Get_TouchedPoint(&touch_pos, strXPT2046_TouchPara)) 
    {
        /* 如果当前处于锁定状态（手指未抬起），直接跳出，不处理逻辑 */
        if (is_touch_locked) return;

        /* --- 触发逻辑：手指刚按下，锁定！ --- */
        is_touch_locked = 1;

        /* 3. 根据当前模式进行分支判断 */
        switch (g_current_mode)
        {
            /* --- 场景：主菜单 --- */
            case MODE_MENU:
                // 1. 点击左侧：进入“综合仪表”
                if (touch_pos.x < GRID_LINE_X && touch_pos.y > TITLE_HEIGHT && touch_pos.y < GRID_LINE_Y) 
                {
                    g_current_mode = MODE_DASHBOARD;
                    Dashboard_Page_Init();           
                    JFC103_SendCmd(JFC_CMD_START);  // 开启采样
                }
                // 2. 点击右侧：进入“实时波形”选择菜单
                else if (touch_pos.x >= GRID_LINE_X && touch_pos.y > TITLE_HEIGHT && touch_pos.y < GRID_LINE_Y) 
                {
                    g_current_mode = MODE_WAVE_SELECT;
                    Waveform_Menu_Init();              
                }
                break;

            /* --- 场景：综合仪表盘 --- */
            case MODE_DASHBOARD:
                // 点击右上角返回按钮
                if (touch_pos.y < TITLE_HEIGHT && touch_pos.x > 200) 
                {
                    JFC103_SendCmd(JFC_CMD_STOP);    // 停止采样
                    g_current_mode = MODE_MENU;      
                    Main_Menu_Display();             
                }
                break;

            /* --- 场景：实时波形选择菜单 --- */
            case MODE_WAVE_SELECT:
                // 1. 返回主菜单
                if (touch_pos.y < TITLE_HEIGHT && touch_pos.x > 200) 
                {
                    g_current_mode = MODE_MENU;      
                    Main_Menu_Display();             
                }
                // 2. 进入心电全屏 (左侧)
                else if (touch_pos.y > TITLE_HEIGHT && touch_pos.x < 160) 
                {
                    g_current_mode = MODE_ECG_LIVE;
                    ECG_Page_Init(); 
                }
                // 3. 进入血氧全屏 (右侧)
                else if (touch_pos.y > TITLE_HEIGHT && touch_pos.x >= 160) 
                {
                    g_current_mode = MODE_SPO2_LIVE;
                    g_HealthData.wave_ptr = 0;       // 进度清零
                    SPO2_Page_Init();                // 初始化全屏背景
                    JFC103_SendCmd(JFC_CMD_START);   // 开启采样
                }
                break;

            /* --- 场景：血氧全屏测量 --- */
            case MODE_SPO2_LIVE:
                // 返回波形选择菜单
                if (touch_pos.y < TITLE_HEIGHT && touch_pos.x > 200) 
                {
                    JFC103_SendCmd(JFC_CMD_STOP);    // 停止采样
                    g_current_mode = MODE_WAVE_SELECT;
                    Waveform_Menu_Init();            
                }
                break;
            case MODE_ECG_LIVE:
                // 点击右上角返回
                if (touch_pos.y < TITLE_HEIGHT && touch_pos.x > 200) 
                {
                    g_current_mode = MODE_WAVE_SELECT;
                    Waveform_Menu_Init();
                }
                break;

            default:
                break;
        }
    }
    else 
    {
        /* --- 释放逻辑：检测到手指离开屏幕，重置锁定标志 --- */
        is_touch_locked = 0;
    }
}

void UI_Display_Handler(void) 
{
    switch (g_current_mode) 
    {
        case MODE_DASHBOARD:
            Dashboard_Page_Update(); // 仪表盘模式下持续刷新数值
            break;
            
        case MODE_ECG_LIVE:
        {   
            // static uint16_t e_draw = 0; 
            // if (++e_draw >= 50) 
            // {
            //     e_draw = 0;
                ECG_Page_Update(); // 每 50 个循环周期执行一次绘制
            // }
            break;
        }
        case MODE_SPO2_LIVE:
        {
            static uint16_t t_draw = 0; 
            if (++t_draw >= 50) 
            {
                t_draw = 0;
                SPO2_Page_Update(); // 每 50 个循环周期执行一次绘制
            }
        }
            break;

        default:
            // 菜单页面为静态，Init 时绘制一次即可，无需在此重复刷新
            break;
    }
}

// 1. 综合仪表盘 Logo (改为更精致的压力/数值表盘感)
void Draw_Icon_Dashboard(uint16_t x, uint16_t y) {
    // 绘制外边框
    LCD_SetTextColor(CYAN);
    ILI9341_DrawRectangle(x - 30, y - 20, 60, 40, 0); 
    // 绘制表盘弧线
    ILI9341_DrawLine(x - 20, y + 10, x + 20, y + 10);
    // 绘制三个不同高度的数值柱状标识
    ILI9341_DrawLine(x - 15, y + 5, x - 15, y - 5);
    ILI9341_DrawLine(x, y + 5, x, y - 10);
    ILI9341_DrawLine(x + 15, y + 5, x + 15, y - 2);
    // 绘制一个小指针点
    LCD_SetTextColor(WHITE);
    ILI9341_SetPointPixel(x + 15, y - 5); 
}

// 2. 波形实时绘制 Logo (经典的监护仪心电感)
void Draw_Icon_Waveform(uint16_t x, uint16_t y) {
    LCD_SetTextColor(GREEN);
    // 绘制一个外框代表监护仪屏幕
    ILI9341_DrawRectangle(x - 35, y - 25, 70, 50, 0);
    // 绘制标准心电波形线
    ILI9341_DrawLine(x - 25, y, x - 15, y);      // P波前基线
    ILI9341_DrawLine(x - 15, y, x - 12, y - 5);  // P波
    ILI9341_DrawLine(x - 12, y - 5, x - 8, y);   
    ILI9341_DrawLine(x - 8, y, x - 5, y + 5);    // Q波
    ILI9341_DrawLine(x - 5, y + 5, x, y - 20);   // R波上
    ILI9341_DrawLine(x, y - 20, x + 5, y + 15);  // S波下
    ILI9341_DrawLine(x + 5, y + 15, x + 10, y);  // 回基线
    ILI9341_DrawLine(x + 10, y, x + 25, y);      // T波后基线
}

// 3. 历史数据回顾 Logo (改为文件夹/列表形式)
void Draw_Icon_History(uint16_t x, uint16_t y) {
    LCD_SetTextColor(YELLOW);
    // 绘制重叠的纸张感
    ILI9341_DrawRectangle(x - 15, y - 20, 35, 45, 0); // 后层纸
    ILI9341_DrawRectangle(x - 20, y - 15, 35, 45, 0); // 前层纸
    // 在前层纸上画几条数据横线
    ILI9341_DrawLine(x - 15, y - 5, x + 10, y - 5);
    ILI9341_DrawLine(x - 15, y + 5, x + 5, y + 5);
    ILI9341_DrawLine(x - 15, y + 15, x + 10, y + 15);
}

// 4. 系统设置 Logo (利用 DrawCircle 绘制更标准的齿轮)
void Draw_Icon_Settings(uint16_t x, uint16_t y) {
    LCD_SetTextColor(GREY);
    // 绘制齿轮主体圆圈
    ILI9341_DrawCircle(x, y, 18, 0); 
    // 绘制中心轴孔
    ILI9341_DrawCircle(x, y, 5, 1); 
    // 绘制八个方向的齿瓣
    ILI9341_DrawLine(x, y - 25, x, y - 18); // 上
    ILI9341_DrawLine(x, y + 18, x, y + 25); // 下
    ILI9341_DrawLine(x - 25, y, x - 18, y); // 左
    ILI9341_DrawLine(x + 18, y, x + 25, y); // 右
    // 绘制斜方向的齿
    ILI9341_DrawLine(x - 18, y - 18, x - 13, y - 13);
    ILI9341_DrawLine(x + 13, y + 13, x + 18, y + 18);
    ILI9341_DrawLine(x + 18, y - 18, x + 13, y - 13);
    ILI9341_DrawLine(x - 13, y + 13, x - 18, y + 18);
}

void Main_Menu_Display(void) {

    // 1. 全屏清除为黑色
    LCD_SetBackColor(BLACK);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    
    // 2. 绘制顶部大标题 "Health-System"
    LCD_SetFont(&Font16x24);
    LCD_SetColors(WHITE, BLACK);
    // 居中计算：(320像素 - 13字符*16像素)/2 = 56
    ILI9341_DispString_EN(56, 10, "Health-System");
    
    // 3. 绘制装饰性分割线
    LCD_SetTextColor(GREY);
    ILI9341_DrawLine(0, TITLE_HEIGHT, 320, TITLE_HEIGHT);      // 标题与菜单的横向分割线
    ILI9341_DrawLine(GRID_LINE_X, TITLE_HEIGHT, GRID_LINE_X, 240); // 垂直分割线
    ILI9341_DrawLine(0, GRID_LINE_Y, 320, GRID_LINE_Y);        // 四宫格水平分割线
    
    // 4. 绘制四个图标（调整 Y 轴中心点以适应新布局）
    Draw_Icon_Dashboard(80, 85);    // 左上中心 (160/2, (140-40)/2 + 40)
    Draw_Icon_Waveform(240, 85);    // 右上中心
    Draw_Icon_History(80, 185);     // 左下中心 (160/2, (240-140)/2 + 140)
    Draw_Icon_Settings(240, 185);   // 右下中心

    // 5. 在图标下方添加文字标注
    LCD_SetFont(&Font8x16);
    
    LCD_SetTextColor(CYAN);
    ILI9341_DispString_CH(48, 115, "综合仪表"); // 每个中文16像素，4个字=64像素
    
    LCD_SetTextColor(GREEN);
    ILI9341_DispString_CH(208, 115, "实时波形");
    
    LCD_SetTextColor(YELLOW);
    ILI9341_DispString_CH(48, 215, "历史回顾");
    
    LCD_SetTextColor(WHITE);
    ILI9341_DispString_CH(208, 215, "系统设置");
}


/**
 * @brief 将数值映射到对应行的 Y 坐标区间
 * @param row 所在的行 (0-3)
 * @param val 当前数值
 * @param min 数值范围最小值
 * @param max 数值范围最大值
 */
uint16_t Map_Value_To_Y(uint8_t row, float val, float min, float max) {
    uint16_t row_top = 41 + (row * 50);    // 每行顶部位
    uint16_t row_bottom = 89 + (row * 50); // 每行底部位
    
    if (val < min) val = min;
    if (val > max) val = max;
    
    // 纵向映射计算：值越大，Y坐标越小（向上跑）
    return (uint16_t)(row_bottom - (val - min) * (row_bottom - row_top) / (max - min));
}


void Dashboard_Page_Init(void) // 综合仪表盘下属页面初始化
{
    /* 1. 清屏并绘制背景 */
    LCD_SetBackColor(BLACK);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    
    /* 2. 绘制顶部状态栏 */
    LCD_SetFont(&Font16x24);
    
    // 绘制左侧标题：综合仪表
    LCD_SetColors(YELLOW, BLACK);
    ILI9341_DispString_CH(50, 10, "综合仪表"); 
    
    // 绘制右上角返回按钮：返回 (使用红色高亮)
    LCD_SetColors(RED, BLACK);
    ILI9341_DispString_CH(260, 10, "返回"); 
    
    /* 3. 绘制分割线 */
    LCD_SetTextColor(GREY);
    ILI9341_DrawLine(0, 40, 320, 40);   // 标题下横线
    ILI9341_DrawLine(0, 90, 320, 90);   // 第一行下
    ILI9341_DrawLine(0, 140, 320, 140); // 第二行下
    ILI9341_DrawLine(0, 190, 320, 190); // 第三行下
    
    // 绘制垂直分割线（区分数值区与趋势区）
    ILI9341_DrawLine(DASH_WAVE_X - 5, 40, DASH_WAVE_X - 5, 240);

    /* 4. 绘制左侧项目标签 */
    LCD_SetFont(&Font8x16);
    LCD_SetTextColor(RED);    ILI9341_DispString_CH(DASH_VAL_X, 55, "心率：");
    LCD_SetTextColor(GREEN);  ILI9341_DispString_CH(DASH_VAL_X, 105, "血氧：");
    LCD_SetTextColor(WHITE);  ILI9341_DispString_CH(DASH_VAL_X, 155, "体温：");
    LCD_SetTextColor(CYAN);   ILI9341_DispString_CH(DASH_VAL_X, 205, "血压：");
}

void Dashboard_Page_Update(void) 
{
    char buf[32];
    const uint16_t x_start = DASH_WAVE_X;             // 起始位置 (140)
    const uint16_t x_end = 315;                       // 结束位置
    const uint16_t x_range = x_end - x_start + 1;     // 总宽度
    uint8_t i; // C89 变量声明
    uint16_t x, clean_x;
    uint16_t current_y[5];
    static uint16_t temp_tick = 0; // 用于体温采样分频
    /* 1. 检查 JFC 模块数据是否就绪 */
    if (JFC_DataReady) 
    {
        /* 根据 JFC103 协议解析数值（假设位置如下，请根据手册微调） */
        g_HealthData.hr      = JFC_RawBuf[65]; /* 心率 */
        g_HealthData.spo2    = JFC_RawBuf[66]; /* 血氧 */
        g_HealthData.bp_high = JFC_RawBuf[71]; /* 高压 */
        g_HealthData.bp_low  = JFC_RawBuf[72]; /* 低压 */
        JFC_DataReady = 0; /* 清除标志位，等待下一包 */
    }
    /* 2. 采样体温数据 (MLX90614) */
    if (++temp_tick >= 50) 
    {
        temp_tick = 0;
        // 读取原始值
        g_HealthData.temp = (float)MLX90614_ReadObjectTempC();
    }

/* 3. 扫描式双血压线绘图逻辑 */
    x = DASH_WAVE_X + g_HealthData.wave_ptr;
    clean_x = x_start + (g_HealthData.wave_ptr + 1) % x_range;
    if (clean_x > x_end) clean_x = x_start + (clean_x - x_end);

    // 计算 Y 映射坐标
    current_y[0] = Map_Value_To_Y(0, (float)g_HealthData.hr, 40, 180);
    current_y[1] = Map_Value_To_Y(1, (float)g_HealthData.spo2, 80, 100);
    current_y[2] = Map_Value_To_Y(2, g_HealthData.temp, 20.0f, 42.0f);
    // 第四行绘制两条线：高压和低压
    current_y[3] = Map_Value_To_Y(3, (float)g_HealthData.bp_high, 40, 160); // 高压映射
    current_y[4] = Map_Value_To_Y(3, (float)g_HealthData.bp_low, 40, 160);  // 低压映射

    // --- 局部擦除 (刷子) ---
    LCD_SetTextColor(BLACK);
    ILI9341_DrawLine(clean_x, 41,  clean_x, 89);
    ILI9341_DrawLine(clean_x, 91,  clean_x, 139);
    ILI9341_DrawLine(clean_x, 141, clean_x, 189);
    ILI9341_DrawLine(clean_x, 191, clean_x, 239);

    // --- 连线绘图 ---
    if (g_HealthData.wave_ptr > 0) {
        LCD_SetTextColor(RED);   ILI9341_DrawLine(x-1, g_HealthData.last_y[0], x, current_y[0]);
        LCD_SetTextColor(GREEN); ILI9341_DrawLine(x-1, g_HealthData.last_y[1], x, current_y[1]);
        LCD_SetTextColor(WHITE); ILI9341_DrawLine(x-1, g_HealthData.last_y[2], x, current_y[2]);
        LCD_SetTextColor(CYAN);  ILI9341_DrawLine(x-1, g_HealthData.last_y[3], x, current_y[3]);
        ILI9341_DrawLine(x-1, g_HealthData.last_y[4], x, current_y[4]); 
    }
    
    // 更新历史坐标
    for(i=0; i<5; i++) g_HealthData.last_y[i] = current_y[i];

    g_HealthData.wave_ptr++;
    if (g_HealthData.wave_ptr >= x_range) {
        g_HealthData.wave_ptr = 0;
    }

    /* 4. 左侧数值按需刷新 (优化 CPU 占用) */
    LCD_SetFont(&Font8x16);

    if (g_HealthData.hr != last_hr) {
        LCD_SetColors(RED, BLACK);
        if (g_HealthData.hr == 0) ILI9341_DispString_EN(DASH_VAL_X + 40, 60, " -- BPM"); // 异常检测
        else { sprintf(buf, "%3d BPM", g_HealthData.hr); ILI9341_DispString_EN(DASH_VAL_X + 40, 60, buf); }
        last_hr = g_HealthData.hr;
    }

    if (g_HealthData.spo2 != last_spo2) {
        LCD_SetColors(GREEN, BLACK);
        if (g_HealthData.spo2 == 0) ILI9341_DispString_EN(DASH_VAL_X + 40, 110, " -- %%  ");
        else { sprintf(buf, "%3d %% ", g_HealthData.spo2); ILI9341_DispString_EN(DASH_VAL_X + 40, 110, buf); }
        last_spo2 = g_HealthData.spo2;
    }

    if (g_HealthData.temp != last_temp) {
        LCD_SetColors(WHITE, BLACK);
        sprintf(buf, "%2.1f C", g_HealthData.temp);
        ILI9341_DispString_EN(DASH_VAL_X + 40, 160, buf);
        last_temp = g_HealthData.temp;
    }

if (g_HealthData.bp_high != last_bp_h || g_HealthData.bp_low != last_bp_l) {
        LCD_SetColors(CYAN, BLACK);
        if (g_HealthData.hr == 0) {
            ILI9341_DispString_EN(DASH_VAL_X + 40, 210, " --/--   "); 
        } else {
            sprintf(buf, "%3d/%2d  ", g_HealthData.bp_high, g_HealthData.bp_low); 
            ILI9341_DispString_EN(DASH_VAL_X + 40, 210, buf);
        }
        last_bp_h = g_HealthData.bp_high; 
        last_bp_l = g_HealthData.bp_low;
    }
}

void Waveform_Menu_Init(void) // 实时波形选择界面
{
    /* 1. 清屏并绘制顶部状态栏 */
    LCD_SetBackColor(BLACK);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    
    LCD_SetFont(&Font16x24);
    
    // 左侧标题：实时波形
    LCD_SetColors(WHITE, BLACK);
    ILI9341_DispString_CH(10, 10, "实时波形"); 
    
    // 右上角：返回
    LCD_SetColors(RED, BLACK);
    ILI9341_DispString_CH(240, 10, "返回"); 
    
    /* 2. 绘制框架 */
    LCD_SetTextColor(GREY);
    ILI9341_DrawLine(0, 40, 320, 40);   // 标题下横线
    ILI9341_DrawLine(160, 40, 160, 240); // 左右二分切割线
    
    /* 3. 绘制左侧：心电测量入口 */
    // 绘制一个心电图小图标
    LCD_SetTextColor(GREEN);
    Draw_Icon_Waveform(80, 100); // 复用之前的波形图标函数
    LCD_SetFont(&Font16x24);
    ILI9341_DispString_CH(48, 160, "心电测量");
    
    /* 4. 绘制右侧：血氧测量入口 */
    // 绘制一个血氧脉冲图标 (假设我们稍微修改下颜色)
    LCD_SetTextColor(CYAN);
    Draw_Icon_Waveform(240, 100); 
    LCD_SetFont(&Font16x24);
    ILI9341_DispString_CH(208, 160, "血氧脉冲");
}

void SPO2_Page_Init(void)   // 血氧全屏测量页面初始化
{
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    
    LCD_SetFont(&Font16x24);
    LCD_SetColors(CYAN, BLACK);
    ILI9341_DispString_CH(10, 10, "血氧脉冲"); 
    
    LCD_SetColors(RED, BLACK);
    ILI9341_DispString_CH(240, 10, "返回"); 
    
    LCD_SetTextColor(GREY);
    ILI9341_DrawLine(0, 40, 320, 40); // 顶部边界

    LCD_SetColors(GREEN, BLACK);
    ILI9341_DispString_CH(80, 210, "血氧值：");
    
    /* 核心状态重置 */
    last_spo2_val = 0xFF; 
    g_HealthData.wave_ptr = 0; // 从 0 开始
    last_y_pos = 120;          // 基准线设在正中间
}

void SPO2_Page_Update(void) {
    /* 1. C89 变量定义置顶 */
    char buf[16];
    uint16_t x, curr_y, clean_x;
    int16_t raw_val;
    uint8_t j; 
    
    /* 静态缓冲区：用于存放尚未画出的点 */
    static int8_t  fifo_queue[256];   /* 增大缓冲区，防止溢出 */
    static uint16_t queue_count = 0;   /* 队伍里还有多少个点 */
    static uint16_t read_idx = 0;      /* 下一个要画的点的位置 */
    static uint16_t write_idx = 0;     /* 采样数据存入的位置 */

    const uint16_t x_start = 10;
    const uint16_t x_end = 310;
    const uint16_t x_range = x_end - x_start;
    const uint16_t baseline_y = 115;   /* 物理中心线 */

    /* 2. 接收数据阶段：只存不画 */
    if (JFC_DataReady) {
        for(j = 0; j < 64; j++) {
            fifo_queue[write_idx] = JFC_RawBuf[1 + j];
            write_idx = (write_idx + 1) % 256;
            if(queue_count < 256) queue_count++; 
        }
        
        /* 刷新数值（数值更新不卡顿，随包走） */
        if (JFC_RawBuf[66] != last_spo2_val) {
            LCD_SetFont(&Font8x16);
            LCD_SetTextColor(BLACK);
            ILI9341_DrawRectangle(176, 210, 80, 24, 1); 
            LCD_SetColors(GREEN, BLACK);
            if (JFC_RawBuf[66] == 0) ILI9341_DispString_EN(176, 210, "-- %");
            else {
                sprintf(buf, "%d %%", JFC_RawBuf[66]);
                ILI9341_DispString_EN(176, 210, buf);
            }
            last_spo2_val = JFC_RawBuf[66];
        }
        JFC_DataReady = 0; 
    }

    /* 3. 绘图阶段：每调用一次 Update，画 1 个像素点 */
    /* 这种“细水长流”的画法，视觉上最丝滑 */
    if (queue_count > 0) {
        x = x_start + g_HealthData.wave_ptr;

        // --- A. 刷子 ---
        clean_x = x_start + (g_HealthData.wave_ptr + 2) % x_range;
        LCD_SetTextColor(BLACK);
        ILI9341_DrawLine(clean_x, 41, clean_x, 205); 

        // --- B. 映射 ---
        raw_val = (int16_t)fifo_queue[read_idx];
        curr_y = baseline_y - (raw_val * 50 / 128); 

        // --- C. 连线 ---
        if (g_HealthData.wave_ptr > 0) {
            LCD_SetTextColor(GREEN);
            ILI9341_DrawLine(x - 1, last_y_pos, x, curr_y);
        }

        // --- D. 状态迭代 ---
        last_y_pos = curr_y;
        read_idx = (read_idx + 1) % 256;
        queue_count--;
        
        g_HealthData.wave_ptr++;
        if (g_HealthData.wave_ptr >= x_range) {
            g_HealthData.wave_ptr = 0; 
        }
    }
}

void ECG_Page_Init(void)   // 心电全屏测量页面初始化
{
    // 1. 全屏清黑
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    
    // 2. 顶部状态栏
    LCD_SetFont(&Font16x24);
    LCD_SetColors(CYAN, BLACK);
    ILI9341_DispString_CH(10, 10, "心电测量"); 
    
    LCD_SetColors(RED, BLACK);
    ILI9341_DispString_CH(240, 10, "返回"); 
    
    LCD_SetTextColor(GREY);
    ILI9341_DrawLine(0, 40, 320, 40); // 标题分割线

    // 3. 变量初始化 (中心点定在120)
    g_HealthData.wave_ptr = 0;
    last_y_pos = 120; 
}

void ECG_Page_Update(void) {
    /* 1. C89 变量定义置顶 */
    uint16_t x, curr_y, clean_x;
    float ecg_voltage;
    int16_t relative_val;
    uint8_t s;
    
    static uint16_t t_draw = 0;

    /* --- 核心配置参数 --- */
    const uint16_t x_start = 10;
    const uint16_t x_end = 310;
    const uint16_t x_range = x_end - x_start;
    
    const uint8_t  x_step = 1;       /* 保持 1 像素步长 */
    const uint16_t baseline_y = 140; /* 基准线设在 135，给 R 波向上留足空间 */

    /* 2. 周期判定：每 10 次主循环绘制一个点 */
    if (++t_draw < 5) return; 
    t_draw = 0;

    /* 3. 获取数据 (AD8232.c 中的滤波接口) */
    ecg_voltage = AD8232_Filter();
    relative_val = (int16_t)(ecg_voltage - 1650.0f);
    
    /* 4. Y 轴映射：为了让峰值更明显，增益维持在 200 以上 */
    curr_y = baseline_y - (relative_val * 150 / 1000); 

    /* --- 核心修正：全屏限幅 --- */
    if (curr_y < 41)  curr_y = 41;  /* 顶部留出标题栏位置 */
    if (curr_y > 239) curr_y = 239; /* 底部直接顶到 ILI9341 的物理边界 */

    /* 5. 绘图逻辑 */
    x = x_start + (g_HealthData.wave_ptr * x_step);

    /* --- 关键修正：动态刷子必须刷到 239 --- */
    /* 如果这里只刷到 205，屏幕下半部分就会留下旧波形的残影，导致“刷新不明白” */
    LCD_SetTextColor(BLACK);
    for(s = 0; s <= x_step; s++) {
        clean_x = x_start + ((g_HealthData.wave_ptr + 2) * x_step) % x_range;
        // 这里的 239 必须和上面的限幅一致，确保垂直方向全覆盖
        ILI9341_DrawLine(clean_x, 41, clean_x, 239); 
    }

    // 连线绘图
    if (g_HealthData.wave_ptr > 0) {
        LCD_SetTextColor(GREEN);
        ILI9341_DrawLine(x - x_step, last_y_pos, x, curr_y);
    }

    /* 6. 状态迭代 */
    last_y_pos = curr_y;
    g_HealthData.wave_ptr++;
    
    if ((g_HealthData.wave_ptr * x_step) >= x_range) {
        g_HealthData.wave_ptr = 0; 
    }
}