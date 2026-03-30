#ifndef __Z_JFC_H
#define __Z_JFC_H

#include "stm32f10x.h"

#define JFC_CMD_START  0x8A  // 开启连续自动发送数据
#define JFC_CMD_STOP   0x88  // 停止发送数据

#pragma pack(1) // 1字节对齐
typedef struct {
    uint8_t  head;          // 第1字节：0xFF 数据头
    int8_t   acdata[64];    // 第2-65字节：心律波形数据
    uint8_t  heartrate;     // 第66字节：心率
    uint8_t  spo2;          // 第67字节：血氧
    uint8_t  bk;            // 第68字节：微循环
    uint8_t  fatigue;       // 第69字节：疲劳指数 (rsv[0])
    uint8_t  rsv1[2];       // 第70-71字节：保留
    uint8_t  systolic;      // 第72字节：收缩压 (rsv[3])
    uint8_t  diastolic;     // 第73字节：舒张压 (rsv[4])
    uint8_t  cardiac_out;   // 第74字节：心输出 (rsv[5])
    uint8_t  resistance;    // 第75字节：外周阻力 (rsv[6])
    uint8_t  rr_period;     // 第76字节：RR周期 (rsv[7])
    uint8_t  sdnn;          // 第77字节：心率变异性 SDNN
    uint8_t  rmssd;         // 第78字节：RMSSD
    uint8_t  nn50;          // 第79字节：NN50
    uint8_t  pnn50;         // 第80字节：PNN50
    uint8_t  rra[6];        // 第81-86字节：RR间期
    uint8_t  rsv2[2];       // 第87-88字节：保留
} JFC103_Data_t;
#pragma pack()

#endif
