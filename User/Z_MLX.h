#ifndef _Z_MLX_H_
#define _Z_MLX_H_

#include "Z_common_headfile.h"
#include "stm32f10x.h"

/* 硬件接线配置 */
#define MLX_SCL_PORT    GPIOB
#define MLX_SCL_PIN     GPIO_Pin_6
#define MLX_SDA_PORT    GPIOB
#define MLX_SDA_PIN     GPIO_Pin_7

/* 寄存器地址 (移植自 Adafruit_MLX90614.h) */
#define MLX90614_I2CADDR 0x5A
#define MLX90614_TA      0x06   // 环境温度
#define MLX90614_TOBJ1   0x07   // 物体1温度

/* GPIO 快速操作 */
#define MLX_SCL_H    GPIO_SetBits(MLX_SCL_PORT, MLX_SCL_PIN)
#define MLX_SCL_L    GPIO_ResetBits(MLX_SCL_PORT, MLX_SCL_PIN)
#define MLX_SDA_H    GPIO_SetBits(MLX_SDA_PORT, MLX_SDA_PIN)
#define MLX_SDA_L    GPIO_ResetBits(MLX_SDA_PORT, MLX_SDA_PIN)
#define MLX_SDA_READ GPIO_ReadInputDataBit(MLX_SDA_PORT, MLX_SDA_PIN)

/* 导出函数 */
void   MLX90614_Init(void);
double MLX90614_ReadObjectTempC(void);
double MLX90614_ReadAmbientTempC(void);

#endif
