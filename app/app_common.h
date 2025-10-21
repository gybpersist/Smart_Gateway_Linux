#ifndef __APP_COMMON_H__
#define __APP_COMMON_H__

/**
 * @brief 获取时间戳
 */
long app_common_getCurrentTime(void);

/**
 * @brief字符数组转为对应的十六进制字符串
 * @paramchars字符数组
 * @paramchars_len字符数组长度
 * @return char* 转换生成的十六进制字符串
 */
char *app_common_charsToHexstr(char *chars, int chars_len);

/**
 * @brief十六进制字符串转为对应的字符数组
 * @paramhex_str十六进制字符串
 * @paramchars_len接收生成的字符数组长度的指针
 * @return char* 转换生成的字符数组
 */
char *app_common_hexstrToChars(char *hex_str, int *chars_len);

#endif /* __APP_COMMON_H__ */
