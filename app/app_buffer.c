#include "app_buffer.h"

#define MAX_MESSAGE_SIZE 255

/**
 * 初始化小缓冲区
 */
static SubBuffer *sub_buffer_init(int total_size)
{
    SubBuffer *sub_buffer = malloc(sizeof(SubBuffer));
    sub_buffer->ptr = malloc(total_size);
    sub_buffer->total_size = total_size;
    sub_buffer->len = 0;

    return sub_buffer;
}

Buffer *app_buffer_init(int total_size)
{
    // 分配缓冲区内存
    Buffer *buffer = malloc(sizeof(Buffer));
    // 初始化两个小缓冲区
    buffer->sub_buffers[0] = sub_buffer_init(total_size);
    buffer->sub_buffers[1] = sub_buffer_init(total_size);
    // 初始化读写索引
    buffer->read_index = 0;  // 默认第一个为读缓冲区
    buffer->write_index = 1; // 默认第二个为写缓冲区
    // 初始化读写锁
    pthread_mutex_init(&buffer->read_lock, NULL);
    pthread_mutex_init(&buffer->write_lock, NULL);

    return buffer;
}

void app_buffer_free(Buffer *buffer)
{
    // 释放两个小缓冲区中的数据内存
    free(buffer->sub_buffers[0]->ptr);
    free(buffer->sub_buffers[1]->ptr);
    // 释放两个小缓冲区内存
    free(buffer->sub_buffers[0]);
    free(buffer->sub_buffers[1]);
    // 释放缓冲区内存
    free(buffer);
}

int app_buffer_write(Buffer *buffer, char *data, int data_len)
{
    // 如果要写入数据长度大于一个消息的最大长度，则返回错误
    if (data_len > MAX_MESSAGE_SIZE)
    {
        log_error("数据长度超过消息数据的最大长度");
        return -1;
    }
    log_debug("开始写数据加锁, data=%.*s", data_len, data);
    // 加锁
    pthread_mutex_lock(&buffer->write_lock);
    // 得到写缓冲区
    SubBuffer *w_buffer = buffer->sub_buffers[buffer->write_index];
    // 如果写缓冲区剩余空间不足，则返回错误
    if (data_len > w_buffer->total_size - w_buffer->len)
    {
        log_error("写缓冲区剩余空间不足");
        // 解锁
        pthread_mutex_unlock(&buffer->write_lock);
        return -1;
    }
    // 将数据写入写缓冲区: 先写长度（一个字节），再写数据
    w_buffer->ptr[w_buffer->len] = data_len;
    memcpy(w_buffer->ptr + w_buffer->len + 1, data, data_len);
    // 更新写缓冲区的长度
    w_buffer->len += data_len + 1;
    // 解锁
    pthread_mutex_unlock(&buffer->write_lock);

    log_debug("写数据成功解锁, data=%.*s", data_len, data);

    return 0;
}

/**
 * 切换读写缓冲区
 */
static void swap_buffer(Buffer *buffer)
{
    log_debug("切换缓冲区, 加锁");
    // 加写锁 =》如果当前正在写数据，不能切换缓冲区，会阻塞等待
    pthread_mutex_lock(&buffer->write_lock);

    int temp = buffer->read_index;
    buffer->read_index = buffer->write_index;
    buffer->write_index = temp;

    // 解写锁
    pthread_mutex_unlock(&buffer->write_lock);
    log_debug("切换缓冲区, 解锁");
}

int app_buffer_read(Buffer *buffer, char *data_buf, int buf_len)
{
    log_debug("开始读数据，加锁");
    // 加锁
    pthread_mutex_lock(&buffer->read_lock);
    // 得到读缓冲区
    SubBuffer *r_buffer = buffer->sub_buffers[buffer->read_index];
    // 如果读缓冲区为空，尝试切换缓冲区
    if (r_buffer->len == 0)
    {
        swap_buffer(buffer);
        r_buffer = buffer->sub_buffers[buffer->read_index];
        // 如果切换后的读缓冲区仍为空，则返回错误
        if (r_buffer->len == 0)
        {
            log_error("读缓冲区为空, 无法读取数据");
            // 解锁
            pthread_mutex_unlock(&buffer->read_lock);
            return -1;
        }
    }
    // 得到读缓冲区中第一个消息的长度
    int data_len = r_buffer->ptr[0];
    // 如果消息长度大于接收容器的长度，则返回错误
    if (data_len > buf_len)
    {
        log_error("消息长度大于接收容器的长度");
        // 解锁
        pthread_mutex_unlock(&buffer->read_lock);
        return -1;
    }
    // 将读缓冲区中的第一个数据帧写入接收容器
    memcpy(data_buf, r_buffer->ptr + 1, data_len);
    // 移除读缓冲区中的第一个数据帧  
    // 将从1+data_len及后面的所有数据移动到0的位置
    memmove(r_buffer->ptr, r_buffer->ptr + 1 + data_len, r_buffer->len - 1 - data_len);
    // 更新读缓冲区的长度
    r_buffer->len -= 1 + data_len;
    // 如果读缓冲区为空，尝试切换缓冲区
    if (r_buffer->len == 0)
    {
        swap_buffer(buffer);
    }
    // 解锁
    pthread_mutex_unlock(&buffer->read_lock);
    log_debug("读数据完成，解锁");
    // 返回读取的消息长度
    return data_len;
}
