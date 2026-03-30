#include "Z_common_headfile.h"

System_Mode_t g_current_mode = MODE_MENU;
strType_XPT2046_Coordinate touch_pos;
HealthData_t g_HealthData;
/* 触控交互与页面跳转逻辑封装 */
void UI_Touch_Handler(void) 
{
    /* C89 变量定义必须置顶 */
    strType_XPT2046_Coordinate touch_pos;

    /* 1. 检测是否有触控按下 */
    if (XPT2046_Get_TouchedPoint(&touch_pos, strXPT2046_TouchPara)) 
    {
        /* 逻辑 A：如果在主菜单，判断进入哪个功能区 */
        if (g_current_mode == MODE_MENU) 
        {
            // 1. 点击左上角“综合仪表”
            if (touch_pos.x < GRID_LINE_X && touch_pos.y > TITLE_HEIGHT && touch_pos.y < GRID_LINE_Y) 
            {
                g_current_mode = MODE_DASHBOARD; // 切换至仪表盘模式
                Dashboard_Page_Init();           // 进入仪表界面
                JFC103_SendCmd(JFC_CMD_START);  //JFC采样开始
            }
            // 2. 点击右上角“实时波形” 
            if (touch_pos.x >= GRID_LINE_X && touch_pos.y > TITLE_HEIGHT && touch_pos.y < GRID_LINE_Y) 
            {
                g_current_mode = MODE_WAVE_SELECT; // 进入波形选择子状态
                Waveform_Menu_Init();              // 绘制子菜单界面
            }
            // 可以在此处添加其他象限的判断（如进入实时波形等）
        }
        /* 如果在综合仪表子页面，点击右上角“返回”区域则回到主菜单 */
        else if (g_current_mode == MODE_DASHBOARD) 
        {
            // 判定是否点击了顶部标题栏 (y < 40) 且位于右侧返回区域 (x > 200)
            if (touch_pos.y < TITLE_HEIGHT && touch_pos.x > 200) 
            {
                JFC103_SendCmd(JFC_CMD_STOP);    //JFC采样停止
                g_current_mode = MODE_MENU;      // 切回菜单模式
                Main_Menu_Display();             // 重新显示四宫格主页面
            }
        }
        /* --- 当前在实时波形选择子菜单 --- */
        else if (g_current_mode == MODE_WAVE_SELECT) 
        {
            // 1. 点击右上角“返回”区域
            if (touch_pos.y < TITLE_HEIGHT && touch_pos.x > 200) 
            {
                g_current_mode = MODE_MENU;      
                Main_Menu_Display();             
            }
            // 2. 点击左侧区域：进入心电全屏绘制
            if (touch_pos.y > TITLE_HEIGHT && touch_pos.x < 160) 
            {
                g_current_mode = MODE_ECG_LIVE;
                // ECG_Page_Init(); // 待编写：心电全屏背景初始化
            }
            // 3. 点击右侧区域：进入血氧全屏绘制
            if (touch_pos.y > TITLE_HEIGHT && touch_pos.x >= 160) 
            {
                g_current_mode = MODE_SPO2_LIVE;
                // SPO2_Page_Init(); // 待编写：血氧全屏背景初始化
            }
        }
        
        // /* 逻辑 C：其他子页面通用返回（可选：点击整个标题栏均可返回） */
        // else if (touch_pos.y < TITLE_HEIGHT) 
        // {
        //     g_current_mode = MODE_MENU;
        //     Main_Menu_Display(); 
        // }
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
    /* 由于 I2C 读取较慢，我们不需要每帧都读，大约每 20-50 帧读一次即可 */
    if (++temp_tick >= 50) 
    {
        temp_tick = 0;
        // 读取物体温度并存入全局结构体
        g_HealthData.temp = (float)MLX90614_ReadObjectTempC(); 
    }
    /* 2. 刷新左侧数值显示 (使用 16x24 字体) */
    LCD_SetFont(&Font8x16);

    // 刷新心率
    LCD_SetColors(RED, BLACK);
    if (g_HealthData.hr == 0) sprintf(buf, " -- BPM"); // 没摸到时显示 --
        else sprintf(buf, "%3d BPM", g_HealthData.hr);
    ILI9341_DispString_EN(DASH_VAL_X + 50, 60, buf);

    // 刷新血氧
    LCD_SetColors(GREEN, BLACK);
    if (g_HealthData.spo2 == 0) sprintf(buf, " -- %%  ");
        else sprintf(buf, "%3d %% ", g_HealthData.spo2);
    ILI9341_DispString_EN(DASH_VAL_X + 50, 110, buf);

    // 刷新体温（暂时显示未接入）
    LCD_SetColors(WHITE, BLACK);
    sprintf(buf, "%2.1f C", g_HealthData.temp); 
    ILI9341_DispString_EN(DASH_VAL_X + 50, 160, buf);

    // 刷新血压
    LCD_SetColors(CYAN, BLACK);
    if (g_HealthData.hr == 0) sprintf(buf, "-/- ");
    else sprintf(buf, "%3d/%3d ", g_HealthData.bp_high, g_HealthData.bp_low);
    ILI9341_DispString_EN(DASH_VAL_X + 50, 210, buf);
    /* 暂时屏蔽波形绘制逻辑，不增加 wave_ptr */
    // g_HealthData.wave_ptr++; 
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
    ILI9341_DispString_CH(208, 160, "血氧测量");
}



