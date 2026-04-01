#include "Z_common_headfile.h"

FATFS sd_fs; // 全局变量，用于挂载 SD 卡
uint16_t g_history_offset = 0; // 当前跳过的记录行数


void SD_Card_Init(void) {
    FRESULT res;
    res = f_mount(&sd_fs, "0:", 1); // 0: 代表 SD 卡，1 代表立即挂载
    if (res == FR_OK) {
        printf("Mount Success!\r\n");
        SD_Test_Process(); // 执行读写闭环测验
    }
}

/**
 * @brief SD卡存取功能深度测验
 * @return 0: 测验失败, 1: 测验成功
 */
uint8_t SD_Test_Process(void) {
    /* 1. C89 变量定义置顶 */
    FIL file;
    FRESULT res;
    uint32_t bw, br;
    char write_data[] = "NEU_Health_Monitor_Test_Success\n"; 
    char read_buf[64];
    
    printf("\r\n--- 开始 SD 卡读写测验 ---\r\n");

    /* 2. 写入测试：创建一个新文件 test.txt */
    // FA_CREATE_ALWAYS: 如果文件存在则覆盖，不存在则创建
    res = f_open(&file, "0:/test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    
    if (res == FR_OK) {
        res = f_write(&file, write_data, strlen(write_data), &bw);
        if (res == FR_OK && bw == strlen(write_data)) {
            printf("1. 写入成功! (已写入 %d 字节)\r\n", bw);
        } else {
            printf("1. 写入失败! 错误码: %d\r\n", res);
            f_close(&file);
            return 0;
        }
        /* 关键：必须关闭文件，数据才会从缓存真正存入物理 SD 卡 */
        f_close(&file);
    } else {
        printf("1. 无法打开文件进行写入! 错误码: %d\r\n", res);
        return 0;
    }

    /* 3. 读取测试：打开刚才的文件并读回 */
    res = f_open(&file, "0:/test.txt", FA_READ);
    if (res == FR_OK) {
        memset(read_buf, 0, sizeof(read_buf)); // 清空缓冲区
        res = f_read(&file, read_buf, sizeof(read_buf) - 1, &br);
        
        if (res == FR_OK && br > 0) {
            printf("2. 读取成功! 读出内容为: %s", read_buf);
            
            // 数据一致性比对
            if (strcmp(read_buf, write_data) == 0) {
                printf("3. 数据校验通过! SD卡读写完全正常。\r\n");
            } else {
                printf("3. 数据校验失败! 读出内容不一致。\r\n");
            }
        }
        f_close(&file);
    } else {
        printf("2. 无法打开文件进行读取! 错误码: %d\r\n", res);
        return 0;
    }

    printf("--- 测验结束 ---\r\n");
    return 1;
}

/**
 * @brief 通用存储接口
 * @param tag 标签：DASH, SPO2, ECG
 */
void SD_Log_Data_With_RTC(const char* tag, const char* content) {
    FIL file;
    FRESULT res;
    UINT bw;
    char write_buf[128];
    
    // 获取当前 RTC 秒数
    uint32_t rtc_sec = RTC_GetCounter();

    res = f_open(&file, "0:/health_log.csv", FA_OPEN_ALWAYS | FA_WRITE);
    if (res == FR_OK) {
        // 移动到文件末尾，防止覆盖
        f_lseek(&file, f_size(&file));

        // 格式：[标签],秒数,内容
        sprintf(write_buf, "[%s],%u,%s\r\n", tag, rtc_sec, content);
        
        f_write(&file, write_buf, strlen(write_buf), &bw);
        f_close(&file);
    }
}

// A. 综合仪表数据存储
void SD_Log_Dashboard(void) {
    char buf[64];
    sprintf(buf, "%d,%d,%.1f,%d/%d", 
            g_HealthData.hr, g_HealthData.spo2, g_HealthData.temp, 
            g_HealthData.bp_high, g_HealthData.bp_low);
    SD_Log_Data_With_RTC("DASH", buf);
}

// B. 血氧波形数据存储
void SD_Log_SPO2_Wave(int16_t raw, uint16_t val) {
    char buf[32];
    sprintf(buf, "%d,%u", raw, val);
    SD_Log_Data_With_RTC("SPO2", buf);
}

// C. 心电原始数据存储
void SD_Log_ECG_Raw(float voltage) {

    char buf[32];
    sprintf(buf, "%.2f", voltage);
    SD_Log_Data_With_RTC("ECG", buf);
}

/**
 * @brief 历史回顾绘图 (零值过滤 + 侧边栏统计)
 */
void SD_Plot_Health_History(History_Type_t type, uint16_t offset) {
    FIL file;
    char line[128], *ptr, buf[16];
    uint32_t t, count = 0, valid_skip = 0;
    int32_t c_hr, d_spo2, f_hi, f_lo;
    float e_temp;
    
    // 统计变量
    float sum1 = 0, max1 = -999.0f, min1 = 999.0f;
    uint16_t x = 10; // 起点稍微左移
    uint16_t curr_y1, last_y1 = 120;
    uint8_t  first_point = 1, isValid = 0;

    // 1. 区域清理 (波形区 5-235, 参数区 240-315)
    LCD_SetTextColor(BLACK);
    ILI9341_DrawRectangle(5, 45, 230, 150, 1); // 波形黑底
    ILI9341_DrawRectangle(240, 45, 75, 150, 1); // 参数黑底
    LCD_SetTextColor(GREY);
    ILI9341_DrawLine(10, 190, 230, 190); // X轴

    if (f_open(&file, "0:/health_log.csv", FA_READ) != FR_OK) return;

    // 2. 遍历数据
    while (f_gets(line, sizeof(line), &file)) {
        if (strstr(line, "DASH") == NULL) continue;
        ptr = strchr(line, ','); if (!ptr) continue;

        if (sscanf(ptr + 1, "%u,%ld,%ld,%f,%d/%d", &t, &c_hr, &d_spo2, &e_temp, &f_hi, &f_lo) >= 5) {
            float val = 0;
            isValid = 0;
            // 零值过滤逻辑
            if (type == HIST_HR && c_hr > 0)     { val = (float)c_hr;   isValid = 1; }
            else if (type == HIST_SPO2 && d_spo2 > 0) { val = (float)d_spo2; isValid = 1; }
            else if (type == HIST_TEMP && e_temp > 20.0f) { val = e_temp;     isValid = 1; }
            else if (type == HIST_BP && f_hi > 0)      { val = (float)f_hi;   isValid = 1; }

            if (!isValid) continue;
            if (valid_skip++ < offset) continue; // 分页

            // 映射公式 (居中且放大)
            switch (type) {
                case HIST_HR:
                    if (c_hr<50) c_hr=50; if (c_hr>110) c_hr=110;
                    curr_y1 = 185 - (uint16_t)((c_hr-50)*130/60);
                    LCD_SetTextColor(RED); break;
                case HIST_SPO2:
                    if (d_spo2<90) d_spo2=90; if (d_spo2>100) d_spo2=100;
                    curr_y1 = 185 - (uint16_t)((d_spo2-90)*130/10);
                    LCD_SetTextColor(GREEN); break;
                case HIST_TEMP:
                    if (e_temp<35.5f) e_temp=35.5f; if (e_temp>38.5f) e_temp=38.5f;
                    curr_y1 = 185 - (uint16_t)((e_temp-35.5f)*130/3.0f);
                    LCD_SetTextColor(YELLOW); break;
                case HIST_BP:
                    if (f_hi<80) f_hi=80; if (f_hi>160) f_hi=160;
                    curr_y1 = 185 - (uint16_t)((f_hi-80)*130/80);
                    LCD_SetTextColor(CYAN); break;
            }

            // 统计
            if (val > max1) max1 = val; if (val < min1) min1 = val;
            sum1 += val; count++;

            // 绘图 (严格限制在 230 像素内)
            if (!first_point && x < 230) {
                ILI9341_DrawLine(x - 5, last_y1, x, curr_y1);
            }
            last_y1 = curr_y1; first_point = 0; x += 5;
        }
        if (x >= 230) break;
    }
    f_close(&file);

    // 3. 右侧参数区绘制 (垂直排列)
    LCD_SetFont(&Font8x16);
    LCD_SetColors(WHITE, BLACK);
    if (count > 0) {
        ILI9341_DispString_EN(242, 60,  "STAT:"); 
        sprintf(buf, "A:%.1f", sum1/count); ILI9341_DispString_EN(242, 90,  buf);
        sprintf(buf, "H:%.0f", max1);       ILI9341_DispString_EN(242, 120, buf);
        sprintf(buf, "L:%.0f", min1);       ILI9341_DispString_EN(242, 150, buf);
    }
}