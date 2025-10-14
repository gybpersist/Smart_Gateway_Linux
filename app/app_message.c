#include "app_message.h"
#include "app_common.h"
#include "cJSON/cJSON.h"
#include "log/log.h"
#include <string.h>
#include "stdlib.h"

/**
* 将字符数组格式的消息数据转换为json格式的消息数据
消息数据包含
连接类型：1 代表蓝牙连接
设备id: "xx"
消息体内容: "abcd"
字符数组格式： 1 2 4 xxabcd
字节1：连接类型
字节2：设备id长度
字节3：消息体长度
字节4~n：设备id和消息体
json格式: {"conn_type": 1, "id": "5858", "msg": "61626364"}
*/

/**
 * @brief 将字符数组格式的消息数据转换为json格式的消息数据
 *
 * @param chars 字符数组格式的消息数据
 * @param chars_len 字符数组长度
 * @return char* json格式的消息数据
 */
char *app_message_charsToJson(char *chars, int chars_len)
{
    // 提取数据
    char connection_type = chars[0];
    int id_len = chars[1];
    int msg_len = chars[2];

    // 把字符数组拷贝到临时变量中
    char id_chars[id_len];
    char msg_chars[msg_len];
    memcpy(id_chars, chars + 3, id_len);
    memcpy(msg_chars, chars + 3 + id_len, msg_len);
    // 把字符数组转换16进制的字符串 //sprintf
    char *id_hex_str = app_common_charsToHexstr(id_chars, id_len);
    char *msg_hex_str = app_common_charsToHexstr(msg_chars, msg_len);

    // 创建空的JSON对象
    cJSON *json = cJSON_CreateObject();
    // 向JSON对象中添加连接类型
    cJSON_AddNumberToObject(json, "conn_type", connection_type);
    // 填入json对象
    cJSON_AddStringToObject(json, "id", id_hex_str);
    cJSON_AddStringToObject(json, "msg", msg_hex_str);
    // 打印出json字符串
    char *json_str = cJSON_PrintUnformatted(json);

    // 释放空间
    cJSON_Delete(json);
    free(id_hex_str);
    free(msg_hex_str);

    return json_str;
}

/**
 * @brief 将json格式的消息数据转换为字符数组格式的消息数据
 *
 * @param json json格式的消息数据
 * @param chars_buff 用来存储字符数组格式的消息数据的容器（空）
 * @param buff_len chars_buff的长度
 * @return int 生成的字符数组的数据长度
 */
int app_message_jsonToChars(char *json, char *chars_buff, int buff_len)
{
    // 解析json字符串
    cJSON *json_obj = cJSON_Parse(json);
    if (json_obj == NULL)
    {
        log_error("json解析失败");
        return -1;
    }

    // 提取json中的字段
    cJSON *conn_type_json = cJSON_GetObjectItem(json_obj, "conn_type");
    cJSON *id_json = cJSON_GetObjectItem(json_obj, "id");
    cJSON *msg_json = cJSON_GetObjectItem(json_obj, "msg");

    // 提取到临时变量
    char conn_type = conn_type_json->valueint;
    char *id_hex_str = id_json->valuestring;
    char *msg_hex_str = msg_json->valuestring;

    int id_len = 0;
    int msg_len = 0;

    // 把id和msg的16进制的字符串转换成字符数组
    char *id_chars = app_common_hexstrToChars(id_hex_str, &id_len);
    char *msg_chars = app_common_hexstrToChars(msg_hex_str, &msg_len);

    // 检查长度
    int len = 3 + id_len + msg_len;
    if (len > buff_len)
    {
        log_error("数据的长度超过缓冲区大小");
        // 释放空间
        free(id_chars);
        free(msg_chars);
        // 释放json对象
        cJSON_Delete(json_obj);
        return -1;
    }

    // 最终组成字符数组
    chars_buff[0] = conn_type;
    chars_buff[1] = id_len;
    chars_buff[2] = msg_len;
    memcpy(chars_buff + 3, id_chars, id_len);
    memcpy(chars_buff + 3 + id_len, msg_chars, msg_len);

    // 释放空间
    free(id_chars);
    free(msg_chars);
    // 释放json对象
    cJSON_Delete(json_obj);

    // 返回字符数组中数据的长度
    return len;
}
