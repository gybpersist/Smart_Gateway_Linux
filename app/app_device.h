#if !defined(__APP_DEVICE_H)
#define __APP_DEVICE_H

#include "app_message.h"
#include "app_buffer.h"
#include <pthread.h>

/**
 * 设备结构体
 */
typedef struct
{
    // 设备文件 接收了下游设备发过来数据的文件
    char *filename;  
    // 文件描述符
    int fd;   
    // 上行缓冲区
    Buffer *up_buffer;
    // 下行缓冲区
    Buffer *down_buffer;
    // 读设备数据的线程
    pthread_t read_thread;
    // 读线程是否运行
    int is_running;
    // 上次写数据的时间
    long last_write_time; 
    // 从设备读取数据后，对数据的处理函数，将数据处理成字符数组消息
    int (*post_read)(		char *data, int len);
    // 写数据前，对数据的处理函数，将字符数组消息处理成设备需要的格式
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

#endif // __APP_DEVICE_H
