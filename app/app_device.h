#ifndef __APP_DEVICE_H__
#define __APP_DEVICE_H__

#include "app_message.h"
#include "app_buffer.h"
#include <pthread.h>

typedef struct
{
    // 1. 蓝牙的日志文件
    char *filename;
    // 2. 蓝牙日志文件的文件描述符
    int fd;
    // 3. 上行缓冲区
    Buffer *up_buffer;
    // 4. 下行缓冲区
    Buffer *down_buffer;
    // 5. 持续读蓝牙日志文件的线程
    pthread_t read_thread;
    // 6. 读蓝牙日志文件的线程是否启用
    int is_running;
    // 7. 上次写蓝牙日志文件的时间
    long last_write_time;
    // 8. 蓝牙日志文件是有特殊格式的 对数据的处理
    int (*post_read)(char *data, int len);
    int (*pre_write)(char *data, int len);
} Device;


/**
 * 初始化设备
 */
Device *app_device_init(char *filename);

/**
 * 启动设备的读线程
 */
int app_device_start();

/**
 * 销毁设备
 */
void app_device_destroy();

#endif /* __APP_DEVICE_H__ */
