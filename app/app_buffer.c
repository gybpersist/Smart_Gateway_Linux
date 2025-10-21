#include "app_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "log/log.h"

/**
 * @brief 初始化小缓冲区
 */
static SubBuffer *sub_buffer_init(int total_size)
{
    SubBuffer *sub_buffer = malloc(sizeof(SubBuffer));

    sub_buffer->ptr = malloc(total_size);
    sub_buffer->total_size = total_size;
    sub_buffer->len = 0;

    return sub_buffer;
}

/**
 * @brief 初始化缓冲区
 * @param total_size 缓冲区总大小
 * @return 返回缓冲区指针
 */
Buffer *app_buffer_init(int total_size)
{
    Buffer *buffer = malloc(sizeof(Buffer));
    buffer->sub_buffers[0] = sub_buffer_init(total_size);
    buffer->sub_buffers[1] = sub_buffer_init(total_size);
    buffer->read_index = 0;
    buffer->write_index = 1;
    pthread_mutex_init(&buffer->read_lock, NULL);
    pthread_mutex_init(&buffer->write_lock, NULL);

    return buffer;
}

/**
 * @brief 释放缓冲区
 * @param buffer 缓冲区指针
 */
void app_buffer_free(Buffer *buffer)
{
    // 释放两个小缓冲区
    free(buffer->sub_buffers[0]->ptr);
    free(buffer->sub_buffers[0]);
    free(buffer->sub_buffers[1]->ptr);
    free(buffer->sub_buffers[1]);

    // 释放缓冲区
    free(buffer);
}

/**
 * @brief 向缓冲区写入数据
 * @param buffer 缓冲区指针
 * @param data 要写入的数据
 * @param data_len 数据长度
 * @return 0：成功，-1：失败
 */
int app_buffer_write(Buffer *buffer, char *data, int data_len)
{
    // 开始写数据
    log_debug("开始写数据并加锁，长度=%d", data_len);

    // 加锁
    pthread_mutex_lock(&buffer->write_lock);

    // 拿到写缓冲区
    SubBuffer *write_sub_buffer = buffer->sub_buffers[buffer->write_index];

    // 判断当前要写的数据长度是否可以写入写缓冲区
    if (data_len > write_sub_buffer->total_size - write_sub_buffer->len)
    {
        log_error("写缓冲区空间不足，写入失败");
        pthread_mutex_unlock(&buffer->write_lock);
        return -1;
    }

    // 先写入一个当前被写入数据的长度
    write_sub_buffer->ptr[write_sub_buffer->len] = data_len;
    // 写数据
    memcpy(write_sub_buffer->ptr + write_sub_buffer->len + 1, data, data_len);
    // 写数据之前改变缓冲区的当前保存的数据长度
    write_sub_buffer->len += data_len + 1;

    // 解锁
    pthread_mutex_unlock(&buffer->write_lock);
    log_debug("写数据完成并解锁，长度=%d", data_len);

    return 0;
}

// 切换读写缓冲区
static void swap_buffer(Buffer *buffer)
{
    log_debug("切换读写缓冲区....");
    // 不需要给读加锁，因为只有读是空的时候 才会切换
    pthread_mutex_lock(&buffer->write_lock);

    // 切换
    int temp = buffer->read_index;
    buffer->read_index = buffer->write_index;
    buffer->write_index = temp;

    // 解锁
    pthread_mutex_unlock(&buffer->write_lock);

    log_debug("切换读写缓冲区完成....");
}

// 读数据
/**
 * @brief 从缓冲区读取数据
 * @param buffer 缓冲区指针
 * @param data_buf 存储读取数据的容器
 * @param buf_len 容器的大小
 * @return 读取的数据长度
 */
int app_buffer_read(Buffer *buffer, char *data_buf, int buf_len)
{
    log_debug("开始读数据....");

    // 加锁
    pthread_mutex_lock(&buffer->read_lock);

    // 拿到读缓冲区
    SubBuffer *read_sub_buffer = buffer->sub_buffers[buffer->read_index];

    // 判断读缓冲区没有数据 则直接切换缓冲区
    if (read_sub_buffer->len == 0)
    {
        log_debug("读缓冲区没有数据，准备切换缓冲区....");
        swap_buffer(buffer);
        // 重新拿新的读缓冲区
        read_sub_buffer = buffer->sub_buffers[buffer->read_index];
        // 再次判断读缓冲区没有数据 则直接返回0
        if (read_sub_buffer->len == 0)
        {
            log_debug("切换缓冲区后读缓冲区仍然没有数据，返回0....");
            pthread_mutex_unlock(&buffer->read_lock);
            return -1;
        }
    }

    // 判断如果缓冲区中消息的长度 大于我接受的容器长度 则直接返回-1
    int msg_len = read_sub_buffer->ptr[0];
    if (msg_len > buf_len)
    {
        log_error("读取缓冲区数据长度大于容器长度，读取失败");
        pthread_mutex_unlock(&buffer->read_lock);
        return -1;
    }

    // 读数据
    memcpy(data_buf, read_sub_buffer->ptr + 1, msg_len);
    // 把后边的数据挪上去
    memmove(read_sub_buffer->ptr, read_sub_buffer->ptr + msg_len + 1, read_sub_buffer->len - (msg_len + 1));

    // 改变读缓冲区中的len长度
    read_sub_buffer->len -= (msg_len + 1);

    // 判断 如果读缓冲区数据读完了 则交换缓冲区
    if (read_sub_buffer->len == 0)
    {
        swap_buffer(buffer);
    }

    // 解锁
    pthread_mutex_unlock(&buffer->read_lock);

    log_debug("读数据完成，长度=%d", msg_len);

    return msg_len;
}
