#define _GNU_SOURCE
#include "app_device.h"
#include "log/log.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "app_pool.h"
#include "app_mqtt.h"
#include "app_common.h"

Device *device;

/**
 * 初始化设备
 */
Device *app_device_init(char *filename)
{
    device = (Device *)malloc(sizeof(Device));

    // 初始化结构体
    device->filename = filename;
    device->fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
    device->up_buffer = app_buffer_init(1024);
    device->down_buffer = app_buffer_init(1024);
    device->is_running = 0;
    device->last_write_time = 0;
    device->post_read = NULL;
    device->pre_write = NULL;

    // 初始化mqtt和线程池
    app_mqtt_init();
    app_pool_init(4);

    return device;
}

static int send_task_func(void *arg)
{
    Device *device = (Device *)arg;
    // 从上行缓冲区读取数据
    char read_buf[128];
    int data_len = app_buffer_read(device->up_buffer, read_buf, sizeof(read_buf));
    log_debug("!!!!!从上行缓冲区读取到的数据长度: %d\n", data_len);
    // 错误处理
    if (data_len == -1)
    {
        log_error("从上行缓冲区读取数据失败\n ");
        return -1;
    }

    // 把读到的数据转换为json字符串数据
    char *json = app_message_charsToJson(read_buf, data_len);
    if (json == NULL)
    {
        log_error("转换为json格式失败\n ");
        return -1;
    }

    // 发送消息到mqtt
    int result = app_mqtt_send(json);
    if (result == -1)
    {
        log_error("发送mqtt消息失败\n ");
        free(json);
        return -1;
    }

    log_debug("发送mqtt消息成功: %s\n", json);
    free(json);
    return 0;
}

// 设备读线程函数
static void *read_thread_func(void *arg)
{
    while (device->is_running)
    {
        // 读取蓝牙日志文件的数据
        char buffer[128];
        ssize_t len = read(device->fd, buffer, 128);

        // log_debug("从蓝牙日志文件读取到的数据长度: %ld\n", len);
        // 判断读到数据 并且蓝牙格式处理函数存在
        if (len > 0 && device->post_read)
        {
            // 调用蓝牙格式处理函数 并更新一下处理后的长度
            len = device->post_read(buffer, len);
        }
        // 拿到处理后的数据了已经
        if (len > 0)
        {
            // 把数据写入到上行缓冲区
            app_buffer_write(device->up_buffer, buffer, len);
            // 将发送消息的操作(放在函数中，作为任务注册给线程池中任务队列)交给线程池处理
            app_pool_registerTask(send_task_func, device);
        }
    }

    return NULL;
}

// 向设备文件写入数据的任务函数
static int write_task_func(void *arg)
{
    // 拿到device
    Device *device = (Device *)arg;

    // 从下行缓冲区拿数据
    char chars_msg[128];
    int msg_len = app_buffer_read(device->down_buffer, chars_msg, sizeof(chars_msg));
    if (msg_len == -1)
    {
        log_error("从下行缓冲区读取数据失败\n ");
        return -1;
    }

    // 对消息数据进行写前处理
    if (device->pre_write)
    {
        msg_len = device->pre_write(chars_msg, msg_len);
    }

    // 蓝牙文件两次写入时间间隔不超过200ms
    long distance = app_common_getCurrentTime() - device->last_write_time;
    if (distance < 200)
    {
        usleep(200000 - distance * 1000);
    }

    // 向文件写入数据
    int write_len = write(device->fd, chars_msg, msg_len);

    if (write_len != msg_len)
    {
        log_error("向设备文件写入数据失败\n ");
        return -1;
    }

    log_debug("向设备文件写入数据成功,写入长度: %d\n", write_len);

    // 更新最后一次写入的时间
    device->last_write_time = app_common_getCurrentTime();

    return 0;
}

static int receive_msg_callback(char *json)
{
    // 把接受到的json数据转为字符数组
    char chars_msg[128];
    int msg_len = app_message_jsonToChars(json, chars_msg, sizeof(chars_msg));

    // 错误处理
    if (msg_len == -1)
    {
        log_error("转换为字符数组格式失败\n ");
        return -1;
    }

    // 把数据交给下行缓冲区
    int result = app_buffer_write(device->down_buffer, chars_msg, msg_len);
    if (result == -1)
    {
        log_error("写入下行缓冲区失败\n ");
        return -1;
    }

    // 将向设备文件写入数据传递给下游设备的工作交给线程池处理
    app_pool_registerTask(write_task_func, device);

    return 0;
}

/**
 * 启动设备的读线程
 */
int app_device_start()
{
    // 判断是否已经启动读线程
    if (device->is_running)
    {
        log_debug("设备已经启动....\n");
        return 0;
    }

    device->is_running = 1;

    // 启动读线程
    pthread_create(&(device->read_thread), NULL, read_thread_func, NULL);

    // 注册收到mqtt消息的回调函数
    app_mqtt_registerRecvCallback(receive_msg_callback);
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