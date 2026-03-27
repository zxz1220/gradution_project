#include "AD8232.h"

// 全局变量
uint16_t filter_buf[FILTER_SIZE] = {0}; // 滑动平均缓冲区
uint8_t filter_idx = 0;                 // 滑动平均索引


/**
 * @brief 滑动平均滤波
 */
uint16_t MovingAverageFilter(uint16_t val)
{
    uint32_t sum = 0;
    uint8_t i = 0;
    
    filter_idx = (filter_idx + 1) % FILTER_SIZE;
    filter_buf[filter_idx] = val;
    
    for(i = 0; i < FILTER_SIZE; i++)
    {
        sum += filter_buf[i];
    }
    
    return (uint16_t)(sum / FILTER_SIZE);
}

/**
 * @brief 50Hz陷波滤波器
 */
float NotchFilter_50Hz(float input)
{
    static float x[3] = {0};
    static float y[3] = {0};
    
    const float a[3] = {1.0000f, -1.4800f, 0.8500f};
    const float b[3] = {0.9250f, -1.4800f, 0.9250f};
    
    x[0] = input;
    y[0] = b[0]*x[0] + b[1]*x[1] + b[2]*x[2] - a[1]*y[1] - a[2]*y[2];
    
    x[2] = x[1]; x[1] = x[0];
    y[2] = y[1]; y[1] = y[0];
    
    return y[0];
}

/**
 * @brief 基线漂移矫正
 */
float BaselineFilter(float input)
{
    static float out_last = 0;
    static float in_last  = 0;
    const float alpha = 0.9955f;
    
    float out = alpha * (out_last + input - in_last);
    in_last = input;
    out_last = out;
    
    return out + (ADC_MAX_VALUE / 2.0f);
}

/**
 * @brief 100Hz低通滤波器
 */
float LowPassFilter_100Hz(float input)
{
    static float x[3] = {0};
    static float y[3] = {0};
    
    const float a[3] = {1.0000f, -0.3200f, 0.1600f};
    const float b[3] = {0.2150f,  0.4300f, 0.2150f};
    
    x[0] = input;
    y[0] = b[0]*x[0] + b[1]*x[1] + b[2]*x[2] - a[1]*y[1] - a[2]*y[2];
    
    x[2] = x[1]; x[1] = x[0];
    y[2] = y[1]; y[1] = y[0];
    
    return y[0];
}

/**
 * @brief ECG总滤波
 */
uint16_t ECG_Filter(uint16_t raw_adc)
{
    float val = (float)raw_adc;
    
    val = BaselineFilter(val);
    val = NotchFilter_50Hz(val);
    val = LowPassFilter_100Hz(val);
    val = (float)MovingAverageFilter((uint16_t)val);
    
    if(val < 0) val = 0;
    if(val > ADC_MAX_VALUE) val = ADC_MAX_VALUE;
    
    return (uint16_t)val;
}

/**
 * @brief AD8232最终滤波接口
 */
float AD8232_Filter(void)
{
    extern __IO uint16_t ADC_ConvertedValue[NOFCHANEL];
    
    uint16_t filtered_adc = ECG_Filter(ADC_ConvertedValue[1]);
    float voltage = filtered_adc * ADC_REF_VOLTAGE / ADC_MAX_VALUE;
    
    return voltage;
}


