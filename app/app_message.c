#include "app_message.h"
#include "app_common.h"
#include "cJSON/cJSON.h"
#include "string.h"
#include <stdlib.h>
#include "log/log.h"
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
char *app_message_charsToJson(char *chars, int chars_len)
{
    // 拿到连接类型
    char cont_type = chars[0];
    // 拿到设备id长度
    int id_len = chars[1];
    // 拿到消息体长度
    int msg_len = chars[2];
    // 创建一个JSON对象
    cJSON *json = cJSON_CreateObject();
    // 向JSON对象中添加连接类型
    cJSON_AddNumberToObject(json, "conn_type", cont_type);

    // 创建两个缓冲区 收集id和msg
    char id_chars[id_len + 1];
    char msg_chars[msg_len + 1];

    // 把设备id和msg的值收集起来
    memcpy(id_chars, chars + 3, id_len);
    memcpy(msg_chars, chars + 3 + id_len, msg_len);

    // 把设备id和msg转换为16进制字符串
    char *id_chars_hex = app_common_charsToHexstr(id_chars, id_len);
    char *msg_chars_hex = app_common_charsToHexstr(msg_chars, msg_len);

    // 把id和msg添加到json对象中
    cJSON_AddStringToObject(json, "id", id_chars_hex);
    cJSON_AddStringToObject(json, "msg", msg_chars_hex);

    // 把json对象转为json字符串
    char *json_str = cJSON_PrintUnformatted(json);

    // 释放内存
    free(id_chars_hex);
    free(msg_chars_hex);
    cJSON_Delete(json);

    return json_str;
}

/**
 * @brief 将json格式的消息数据转换为字符数组格式的消息数据
 * @param jsonjson格式的消息数据
 * @param chars_buff用来存储字符数组格式的消息数据的容器（空）
 * @return 生成的字符数组的数据长度
 */
int app_message_jsonToChars(char *json, char *chars_buff, int buff_len)
{
    // 把收到的json字符串解析为json对象
    cJSON *json_obj = cJSON_Parse(json);
    // 判断是否解析成功
    if (json_obj == NULL)
    {
        return -1;
    }

    // 从json对象中获取连接类型 id 和 数据
    cJSON *conn_type_item = cJSON_GetObjectItem(json_obj, "conn_type");
    cJSON *id_item = cJSON_GetObjectItem(json_obj, "id");
    cJSON *msg_item = cJSON_GetObjectItem(json_obj, "msg");

    // 取conn_type的值
    char conn_type = conn_type_item->valueint;
    // 取id的值
    char *id_hex_str = id_item->valuestring;
    // 取msg的值
    char *msg_hex_str = msg_item->valuestring;

    // 定义两个变量 用来接收转换生成的字符数组长度
    int id_len = 0;
    int msg_len = 0;

    // 把id和msg的16进制字符串转换为字符数组
    char *id_chars = app_common_hexstrToChars(id_hex_str, &id_len);
    char *msg_chars = app_common_hexstrToChars(msg_hex_str, &msg_len);

    // 计算最终生成的字符数组的长度
    int len = 3 + id_len + msg_len;

    // 判断buff_len是否足够存储生成的字符数组
    if (len > buff_len)
    {
        log_error("数据的长度大于缓冲区的长度");
        free(id_chars);
        free(msg_chars);
        cJSON_Delete(json_obj);
        return -1;
    }

    // 拼接最终的字符串
    chars_buff[0] = conn_type;
    chars_buff[1] = id_len;
    chars_buff[2] = msg_len;
    memcpy(chars_buff + 3, id_chars, id_len);
    memcpy(chars_buff + 3 + id_len, msg_chars, msg_len);

    // 释放空间
    free(id_chars);
    free(msg_chars);
    cJSON_Delete(json_obj);

    return len;
}