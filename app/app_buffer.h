#ifndef __APP_BUFFER_H__
#define __APP_BUFFER_H__

#include <pthread.h>
#include "log/log.h"
#include <string.h>
#include <stdlib.h>

// 小缓冲区
typedef struct
{
    unsigned char *ptr;      // 数据指针
    int total_size;          // 空间大小
    int len;                 // 数据长度
} SubBuffer;

// 大缓冲区
typedef struct
{
    SubBuffer *sub_buffers[2]; // 读和写缓冲区的数组
    int read_index;           // 读缓冲区索引
    int write_index;          // 写缓冲区索引
    pthread_mutex_t read_lock;  // 读锁
    pthread_mutex_t write_lock; // 写锁
} Buffer;

/**
 * @brief 初始化缓冲区
 * @param total_size 缓冲区总大小
 * @return 返回缓冲区指针
 */
Buffer *app_buffer_init(int total_size);

/**
 * @brief 释放缓冲区
 * @param buffer 缓冲区指针
 */
void app_buffer_free(Buffer *buffer);

/**
 * @brief 向缓冲区写入数据
 * @param buffer 缓冲区指针
 * @param data 要写入的数据
 * @param data_len 数据长度
 * @return 0：成功，-1：失败
 */
int app_buffer_write(Buffer *buffer, char *data, int data_len);

/**
 * @brief 从缓冲区读取数据
 * @param buffer 缓冲区指针
 * @param data_buf 存储读取数据的容器
 * @param buf_len 容器的大小
 * @return 读取的数据长度
 */
int app_buffer_read(Buffer *buffer, char *data_buf, int buf_len);

#endif /* __APP_BUFFER_H__ */
