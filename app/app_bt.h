#ifndef __APP_BT_H__
#define __APP_BT_H__

#include "app_device.h"

/**
 * 初始化
 * 1. 将蓝牙数据的预处理方法装配给设备模块
 * 2 设置设备的蓝牙默认配置，并连接蓝牙
 */
int app_bt_init(Device *device);

/**
 * 写前预处理
 */
int app_bt_preWrite(char *data, int len);

/**
 *  读后预处理
 */
int app_bt_postRead(char *data, int len);

#endif /* __APP_BT_H__ */
