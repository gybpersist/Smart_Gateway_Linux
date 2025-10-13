#include "cJSON/cJSON.h"
#include "log/log.h"
#include <stdlib.h>

/**
测试 cJSON库
1. 生成json字符串
2. 解析json字符串
*/
int main(int argc, char const *argv[])
{
    // 1. 生成json字符串
    // 创建一个空的JSON对象
    cJSON *root = cJSON_CreateObject();
    // 添加一个字符串类型的键值对
    cJSON_AddStringToObject(root, "name", "John");
    // 添加一个整数类型的键值对
    cJSON_AddNumberToObject(root, "age", 30);
    // 将JSON对象转换为字符串
    char *json_str = cJSON_PrintUnformatted(root);
    log_debug("生成json字符串:%s", json_str);

    // 2. 解析json字符串， 得到其中包含的各个数据
    cJSON *root2 = cJSON_Parse(json_str);
    if (root2 == NULL)
    {
        log_debug("解析json失败");
        return -1;
    }
    cJSON *nameJSON = cJSON_GetObjectItem(root2, "name");
    cJSON *ageJSON = cJSON_GetObjectItem(root2, "age");
    if (nameJSON == NULL || ageJSON == NULL)
    {
        log_debug("解析json中的name或age失败");
        return -1;
    }
    char *name = nameJSON->valuestring;
    int age = ageJSON->valueint;
    log_debug("解析json字符串:name=%s, age=%d", name, age);

    // 删除JSON对象及其所有子对象
    cJSON_Delete(root);
    cJSON_Delete(root2);
    // 释放cjson生成的json字符串
    free(json_str);

    return 0;
}
