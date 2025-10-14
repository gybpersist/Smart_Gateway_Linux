#ifndef __APP_COMMON_H__
#define __APP_COMMON_H__

/**
 * @brief获取自1970年开始的时间戳，以ms为单位
 *
 * @return long 时间戳
 */
long app_common_getCurrentTime(void);

/**
 * @brief 字符数组转换为十六进制字符串
 * 
 * @param chars 字符数组
 * @param chars_len 字符数组长度
 * @return char* 转换生成的十六进制字符串
 */
char* app_common_hexstrToChars(char *hex_str, int *chars_len);

/**
 * @brief 十六进制字符串转换为对应的字符数组
 * 
 * @param hex_str 十六进制字符串
 * @param chars_len 接收字符数组长度的指针
 * @return char* 转换生生成的字符数组
 */
char *app_common_charsToHexstr(char *chars, int chars_len);

#endif /* __APP_COMMON_H__ */
