#ifndef __APP_MESSAGE_H__
#define __APP_MESSAGE_H__

/**
* 将字符数组格式的消息数据转换为json格式的消息数据
    消息数据包含
    连接类型：1 代表蓝牙连接
    设备id: "xx" 
    消息体内容: "abcd"
    字符数组格式： 124xxabcd
    字节1：连接类型
    字节2：设备id长度
    字节3：消息体长度
    字节4~n：设备id和消息体
    json格式: {"conn_type": 1, "id": "5858", "msg": "61626364"}

*/
/**
 * @brief 将字符数组格式的消息数据转换为json格式的消息数据
 * @param chars字符数组格式的消息数据
 * @param chars_len字符数组长度
 * @return json格式的消息数据
 *
 */
char *app_message_charsToJson(char *chars, int chars_len);

/**
 * @brief 将json格式的消息数据转换为字符数组格式的消息数据
 * @param jsonjson格式的消息数据
 * @param chars_buff用来存储字符数组格式的消息数据的容器（空）
 * @return 生成的字符数组的数据长度
 */
int app_message_jsonToChars(char *json, char *chars_buff, int buff_len);

#endif /* __APP_MESSAGE_H__ */
