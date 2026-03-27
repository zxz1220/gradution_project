#include "Z_MLX.h"

/* --- 私有底层：GPIO 方向控制 --- */
static void MLX_SDA_Mode(uint8_t out) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = MLX_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = out ? GPIO_Mode_Out_PP : GPIO_Mode_IPU;
    GPIO_Init(MLX_SDA_PORT, &GPIO_InitStructure);
}

/* --- IIC 基础信号 --- */
static void MLX_Start(void) {
    MLX_SDA_Mode(1);
    MLX_SDA_H; MLX_SCL_H; delay_us(5);
    MLX_SDA_L; delay_us(5);
    MLX_SCL_L;
}

static void MLX_Stop(void) {
    MLX_SDA_Mode(1);
    MLX_SCL_L; MLX_SDA_L; delay_us(5);
    MLX_SCL_H; MLX_SDA_H; delay_us(5);
}

static uint8_t MLX_WaitAck(void) {
    uint16_t t = 0;
    MLX_SDA_Mode(0);
    MLX_SCL_H; delay_us(2);
    while(MLX_SDA_READ) {
        if(++t > 500) { MLX_Stop(); return 1; }
    }
    MLX_SCL_L;
    return 0;
}

static void MLX_SendByte(uint8_t byte) {
    uint8_t i;
    MLX_SDA_Mode(1);
    for(i=0; i<8; i++) {
        MLX_SCL_L;
        if(byte & 0x80) MLX_SDA_H; else MLX_SDA_L;
        byte <<= 1;
        delay_us(2);
        MLX_SCL_H;
        delay_us(2);
    }
    MLX_SCL_L;
}

static uint8_t MLX_ReadByte(uint8_t ack) {
    uint8_t i, res = 0;
    MLX_SDA_Mode(0);
    for(i=0; i<8; i++) {
        MLX_SCL_L; delay_us(2);
        MLX_SCL_H;
        res <<= 1;
        if(MLX_SDA_READ) res++;
        delay_us(2);
    }
    MLX_SCL_L;
    MLX_SDA_Mode(1);
    if(ack) MLX_SDA_L; else MLX_SDA_H;
    delay_us(2);
    MLX_SCL_H; delay_us(2);
    MLX_SCL_L;
    return res;
}

/* --- 核心算法移植自 Adafruit --- */

/**
 * 对应 Adafruit_MLX90614::read16(uint8_t a)
 */
static uint16_t MLX90614_Read16(uint8_t reg) {
    uint16_t ret;
    uint8_t pec;

    MLX_Start();
    MLX_SendByte(MLX90614_I2CADDR << 1); // Wire.beginTransmission(_addr)
    MLX_WaitAck();
    
    MLX_SendByte(reg); // Wire.write(a)
    MLX_WaitAck();

    MLX_Start(); // Wire.endTransmission(false) + 重启信号
    MLX_SendByte((MLX90614_I2CADDR << 1) | 0x01); // Wire.requestFrom
    MLX_WaitAck();

    ret = MLX_ReadByte(1);      // Low byte
    ret |= (MLX_ReadByte(1) << 8); // High byte
    pec = MLX_ReadByte(0);      // PEC
    
    MLX_Stop();
    return ret;
}

/**
 * 对应 Adafruit_MLX90614::readTemp(uint8_t reg)
 */
static float MLX90614_ReadTemp(uint8_t reg) {
    float temp;
    temp = MLX90614_Read16(reg); // 读取原始16位数据
    temp *= 0.02f;               // Adafruit: temp *= .02
    temp -= 273.15f;             // Adafruit: temp -= 273.15
    return temp;
}

/* --- 对外公共接口 --- */

void MLX90614_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = MLX_SCL_PIN | MLX_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MLX_SCL_PORT, &GPIO_InitStructure);
    
    MLX_SCL_H;
    MLX_SDA_H;
}

double MLX90614_ReadObjectTempC(void) //物体温度
{
    return (double)MLX90614_ReadTemp(MLX90614_TOBJ1);
}

double MLX90614_ReadAmbientTempC(void) //环境温度
{
    return (double)MLX90614_ReadTemp(MLX90614_TA);
}