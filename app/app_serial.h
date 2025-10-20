#ifndef __APP_SERIAL_H__
#define __APP_SERIAL_H__

#include <termios.h>
#include "app_device.h"

// 波特率枚举
typedef enum
{
    BR_9600 = B9600,
    BR_115200 = B115200,
} BraudRate;

// 校验位枚举
typedef enum
{
    Parity_NONE = 0,
    Parity_ODD = PARENB | PARODD,
    Parity_EVEN = PARENB,
} Parity;

// 停止位枚举
typedef enum
{
    SB_1 = 0,
    SB_2 = CSTOPB,
} StopBits;

/**
 * 初始化
 */
int app_serial_init(Device *device);

/**
 * 设置波特率
 */
int app_serial_setBraudRate(Device *device, BraudRate rate);

/**
 * 设置校验位
 */
int app_serial_setParity(Device *device, Parity parity);

/**
 * 设置停止位
 */
int app_serial_setStopBits(Device *device, StopBits stopBits);

/**
 * 设置是否阻塞模式
 */
int app_serial_setBlock(Device *device, int is_block);

/**
 * 设置为原始模式
 */
int app_serial_setRaw(Device *device);

#endif /* __APP_SERIAL_H__ */
