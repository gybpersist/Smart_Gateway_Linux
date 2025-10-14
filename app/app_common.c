#include "app_common.h"
#include <stdlib.h>   // NULL
#include <sys/time.h> // timeval / gettimeofday()
#include <string.h>   // strlen()
#include <stdio.h> // sprintf()

/**
 * @brief获取自1970年开始的时间戳，以ms为单位
 *
 * @return long 时间戳
 */
long app_common_getCurrentTime(void)
{
    // 获取当前时间，单位为秒和微秒
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // tv_sec: 秒  tv_usec: 微秒
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/**
 * @brief 字符数组转换为十六进制字符串
 * 
 * @param chars 字符数组
 * @param chars_len 字符数组长度
 * @return char* 转换生成的十六进制字符串
 */
char *app_common_charsToHexstr(char *chars, int chars_len)
{
    // 动态申请内存
    char *hex_str = (char *)malloc(chars_len * 2 + 1);
    // 循环遍历
    for (int i = 0; i < chars_len; i++)
    {
        // 每个字节转换为2个十六进制字符
        sprintf(hex_str + i * 2, "%02x", chars[i]);
    }
    // 字符串结尾
    hex_str[chars_len * 2] = '\0';

    return hex_str;
}

/**
 * @brief 十六进制字符串转换为对应的字符数组
 * 
 * @param hex_str 十六进制字符串
 * @param chars_len 接收字符数组长度的指针
 * @return char* 转换生生成的字符数组
 */
char *app_common_hexstrToChars(char *hex_str, int *chars_len)
{
    // 计算16进制字符串的长度
    int hex_str_len = strlen((const char *)hex_str);
    // 计算转换后的字符串的长度
    *chars_len = hex_str_len / 2;
    // 动态申请内存
    char *chars_str = (char *)malloc(*chars_len);
    // 循环遍历
    for (int i = 0; i < hex_str_len; i += 2)
    {
        // 每个十六进制字符转换为1个字节
        sscanf(hex_str + i, "%02x", (unsigned int *)(chars_str + i / 2));
    }

    return chars_str;
}
