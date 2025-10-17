#include "app_buffer.h"
#include "log/log.h"

int main(int argc, char const *argv[])
{
    Buffer *buffer = app_buffer_init(12);

    // 写入数据1 2
    app_buffer_write(buffer, "abcde", 5);
    app_buffer_write(buffer, "fghijk", 6);

    // 读取数据1
    char read_buf[100];
    int data_len = app_buffer_read(buffer, read_buf, 100);
    log_debug("读数据1: %.*s, len: %d", data_len, read_buf, data_len);

    // 写入数据3
    app_buffer_write(buffer, "123456", 6);

    // 读取数据2
    char read_buf2[100];
    int data_len2 = app_buffer_read(buffer, read_buf2, 100);
    log_debug("读数据2: %.*s, len2: %d", data_len2, read_buf2, data_len2);

    // 读数据3
    char read_buf3[100];
    int data_len3 = app_buffer_read(buffer, read_buf3, 100);
    log_debug("读数据3: %.*s, len: %d", data_len3, read_buf3, data_len3);

    // 写数据 => 超出缓冲区大小，失败
    app_buffer_write(buffer, "1234567890abcd", 14);
    return 0;
}  
