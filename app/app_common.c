#include "app_common.h"
#include <stdlib.h>   // NULL
#include <sys/time.h> // timeval / gettimeofday
#include <string.h>
#include <stdio.h> // sprintf

/**
 * @brief 获取时间戳
 */
long app_common_getCurrentTime(void)
{

    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/**
 * @brief字符数组转为对应的十六进制字符串
 * @paramchars字符数组
 * @paramchars_len字符数组长度
 * @return char* 转换生成的十六进制字符串
 */
char *app_common_charsToHexstr(char *chars, int chars_len)
{
    // 动态分配一个16进制字符串空间
    char *hex_str = (char *)malloc(chars_len * 2 + 1);
    // 遍历chars字符串 拿到每一个字符 得到对应的两位十六进制字符串 并添加到hex_str中
    for (int i = 0; i < chars_len; i++)
    {
        sprintf(hex_str + i * 2, "%02X", chars[i]);
    }
    // 在hex_str的末尾添加字符串结束符
    hex_str[chars_len * 2] = '\0';

    return hex_str;
}

/**
 * @brief十六进制字符串转为对应的字符数组
 * @paramhex_str十六进制字符串
 * @paramchars_len接收生成的字符数组长度的指针
 * @return char* 转换生成的字符数组
 */
char *app_common_hexstrToChars(char *hex_str, int *chars_len)
{
    // 获取hex_str的长度
    int hex_str_len = strlen(hex_str);
    // 计算最终生成字符串的长度
    *chars_len = hex_str_len / 2;
    // 动态分配一个字符数组空间
    char *chars = (char *)malloc(*chars_len + 1);
    // 遍历hex_str字符串 每两个十六进制字符转换为一个字符 并添加到chars中
    for (int i = 0; i < hex_str_len; i += 2)
    {
        sscanf(hex_str + i, "%02X", (unsigned int *)(chars + i / 2));
    }
    //给chars添加字符串结束符
    chars[*chars_len] = '\0';

    return chars;
}