#ifndef __APP_MESSAGE_H__
#define __APP_MESSAGE_H__

/**
 * @brief 将字符数组格式的消息数据转换为json格式的消息数据
 * 
 * @param chars 字符数组格式的消息数据
 * @param chars_len 字符数组长度
 * @return char* json格式的消息数据
 */
char* app_message_charsToJson(char* chars, int chars_len);

/**
 * @brief 将json格式的消息数据转换为字符数组格式的消息数据
 * 
 * @param json json格式的消息数据
 * @param chars_buff 用来存储字符数组格式的消息数据的容器（空）
 * @param buff_len chars_buff的长度
 * @return int 生成的字符数组的数据长度
 */
int app_message_jsonToChars(char* json, char* chars_buff, int buff_len);

#endif /* __APP_MESSAGE_H__ */
