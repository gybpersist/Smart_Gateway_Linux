#define _GNU_SOURCE
#include "app_device.h"
#include "log/log.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "app_pool.h"
#include "app_mqtt.h"
#include "app_common.h"

static Device *device;

Device *app_device_init(char *filename)
{
    // 为设备分配内存
    device = (Device *)malloc(sizeof(Device));

    // 初始化设备的属性
    device->filename = filename;
    device->fd = open(filename, O_RDWR);
    device->up_buffer = app_buffer_init(1024);
    device->down_buffer = app_buffer_init(1024);
    device->is_running = 0;
    device->last_write_time = 0;
    device->post_read = NULL;
    device->pre_write = NULL;

    // 初始化线程池
    app_pool_init(4);

    // 初始化mqtt
    app_mqtt_init();

    return device;
}

/**
 * 发送消息的任务函数 =》 由线程池线程在分线程执行
 */
static int send_task_func(void *arg)
{
    // Device *device = (Device *)arg;
    // 从上行缓冲区读取一帧数据（也就是字符数组消息）
    char read_buf[128];
    int data_len = app_buffer_read(device->up_buffer, read_buf, sizeof(read_buf));
    if (data_len == -1)
    {
        log_error("从上行缓冲区读取数据失败");
        return -1;
    }
    // 将字符数组消息转换为JSON消息
    char *json = app_message_charsToJson(read_buf, data_len);
    if (json == NULL)
    {
        log_error("字符数组消息转换为JSON消息失败");
        return -1;
    }
    // 发送消息到MQTT服务器
    int result = app_mqtt_send(json);
    if (result == -1)
    {
        log_error("发送消息到MQTT服务器失败");
        return -1;
    }
    log_debug("发送消息到MQTT服务器成功: %s", json);

    // 释放内存
    free(json);

    return 0;
}

/**
 * 不断读取设备文件的线程函数
 */
static void *read_thread_func(void *arg)
{
    while (device->is_running)
    {
        // 读取设备文件
        char buffer[128];
        int len = read(device->fd, buffer, sizeof(buffer));

        // 对读取的设备数据进行读后处理
        if (len > 0 && device->post_read)
        {
            log_debug("从设备文件读取数据: %.*s", len, buffer);
            len = device->post_read(buffer, len);
        }
        if (len > 0)
        {
            // 将读取的数据写入到上行缓冲区
            app_buffer_write(device->up_buffer, buffer, len);

            // 将发送消息的操作交给线程池处理
            app_pool_registerTask(send_task_func, device);
        }
    }

    return NULL;
}

/**
 * 向设备文件写入数据的任务函数 =》 由线程池线程在分线程执行
 */
static int write_task_func(void *arg)
{
    // Device *device = (Device *)arg;
    // 从下行缓冲区读取一帧数据（也就是字符数组消息）
    char chars_msg[128];
    int msg_len = app_buffer_read(device->down_buffer, chars_msg, sizeof(chars_msg));

    if (msg_len == -1)
    {
        log_error("从下行缓冲区读取数据失败");
        return -1;
    }

    // 对消息数据进行写前处理
    if (device->pre_write)
    {
        msg_len = device->pre_write(chars_msg, msg_len);
        if (msg_len == -1)
        {
            log_error("消息数据写前处理失败");
            return -1;
        }
    }

    // 限制两次写入间隔时间不能小于200ms
    long distance = app_common_getCurrentTime() - device->last_write_time;
    if (distance < 200)
    {
        usleep(200000 - distance * 1000);
    }

    // 向设备文件写入数据
    int write_len = write(device->fd, chars_msg, msg_len);

    if (write_len != msg_len)
    {
        log_error("向设备文件写入数据失败");
        return -1;
    }
    log_debug("向设备文件写入数据成功: %.*s", msg_len, chars_msg);

    // 更新最后写入时间
    device->last_write_time = app_common_getCurrentTime();

    return 0;
}

/**
 * 设备接收远程消息的回调函数
 */
static int receive_msg_callback(char *json)
{
    // 将接收到的JSON消息转换为字符数组消息
    char chars_msg[128];
    int msg_len = app_message_jsonToChars(json, chars_msg, sizeof(chars_msg));
    if (msg_len == -1)
    {
        log_error("JSON消息转换为字符数组消息失败");
        return -1;
    }
    // 将字符数组消息写入到下行缓冲区
    int result = app_buffer_write(device->down_buffer, chars_msg, msg_len);
    if (result == -1)
    {
        log_error("将字符数组消息写入到下行缓冲区失败");
        return -1;
    }

    // 将向设备文件写入数据传递给下游设备的操作交给线程池处理
    app_pool_registerTask(write_task_func, device);

    return 0;
}

int app_device_start()
{
    if (device->is_running == 1)
    {
        log_debug("设备已经运行中");
        return 0;
    }
    device->is_running = 1; // 设置设备为运行状态
    // 启动读设备文件的线程
    pthread_create(&device->read_thread, NULL, read_thread_func, NULL);
    // 注册接收远程消息的回调处理函数
    app_mqtt_registerRecvCallback(receive_msg_callback);

    return 0;
}

void app_device_destroy()
{
    // 释放上行和下行缓冲区
    app_buffer_free(device->up_buffer);
    app_buffer_free(device->down_buffer);
    // 关闭设备文件
    close(device->fd);
    // 取消读线程
    pthread_cancel(device->read_thread);
    // 释放设备内存
    free(device);

    // 关闭线程池
    app_pool_close();

    // 关闭mqtt
    app_mqtt_close();
}
