#ifndef __APP_BT_H__
#define __APP_BT_H__

#include "app_device.h"

typedef enum {
    BT_BAUD_9600 = '4',
    BT_BAUD_115200 = '8'
} BTBraudRate;

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

/**
 * 测试蓝牙是否可用
 */
int app_bt_status(Device *device);

/**
 * 修改蓝牙名称
 */
int app_bt_rename(Device *device, char *name);

/**
 * 设置波特率
 */
int app_bt_setBaudRate(Device *device, BTBraudRate braud_rate);

/**
 * 重置(修改的配置才生效)
 */
int app_bt_reset(Device *device);

/**
 * 设置组网 id(同一个组的多个设备组网 id 相同)
 * net_id：4 位十六进制字符串
 */
int app_bt_setNetId(Device *device, char *net_id);

/**
 * 设置蓝牙 MAC 地址(同一个组的多个设备 MAC 地址不同)
 * maddr: 4 位十六进制字符串
 */
int app_bt_setMAddr(Device *device, char *maddr);

#endif /* __APP_BT_H__ */
