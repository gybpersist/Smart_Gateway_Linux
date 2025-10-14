#include "app_message.h"
#include "log/log.h"

int main(int argc, char const *argv[])
{
    char *json_str = "{\"conn_type\":1,\"id\":\"5858\",\"msg\":\"61626364\"}";

    // json格式消息转为字符数组格式消息
    char chars_buff[100];
    int chars_len = app_message_jsonToChars(json_str, chars_buff, 100);
    log_info("chars_len:%d ,chars_data:%.*s", chars_len, chars_len, chars_buff);

    // 字符数组格式消息转为json格式消息
    char *json = app_message_charsToJson(chars_buff, chars_len);
    log_info("json: %s", json);

    return 0;
}
