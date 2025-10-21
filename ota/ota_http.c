#include "ota_http.h"
#include "log/log.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief curl回调函数
 * @param data 请求获得的数据
 * @param size 每次请求的数据大小
 * @param nmemb 每次请求的数据个数
 * @param clientp 自定义数据容器
 */
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    char *data = (char *)ptr;
    char *json_buf = (char *)userp;
    size_t data_len = size * nmemb;

    memcpy(json_buf, data, data_len);
    json_buf[data_len] = '\0';

    return data_len;
}

/**
 * 访问url，获取json数据
 * 1. 初始化curl
 * 2. 设置url
 * 3. 设置获取响应数据的回调函数
 * 4. 设置回调函数接收数据的参数
 * 5. 执行http请求
 * 6. 释放curl资源
 * 7. 返回json数据
 */
char *ota_http_getJson(char *url)
{
    // 初始化CURL
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        log_error("curl_easy_init failed");
        return NULL;
    }
    
    // 设置URL
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // 设置回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    
    // 指定要传递给回调函数的参数
    char *json = malloc(1024);
    if (json == NULL) {
        log_error("malloc failed");
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)json);
    
    // 执行请求
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        log_error("请求出错: %s", curl_easy_strerror(res));
        free(json);
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    // 清理CURL
    curl_easy_cleanup(curl);
    // 返回响应体
    return json;
}

/**
 * 访问url, 下载文件到指定位置
 * 1. 初始化curl
 * 2. 设置url
 * 3. 设置获取文件数据的回调函数 fwrite
 * 4. 设置回调函数接收文件数据的参数 FILE指针 => 文件数据会自动保存到对应的文件中
 * 5. 执行http请求
 * 6. 释放curl资源
 * 7. 返回0
 */
int ota_http_download(char *url, char *file_path)
{
    // 1 初始化一个curl 
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        log_error("curl_easy_init error");
        return -1;
    }
    
    // 2 给curl设置参数 1 url 2 接收数据的回调函数 3 接收数据的容器
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);

    FILE *file = fopen(file_path, "wb");
    if (file == NULL) {
        log_error("fopen error: %s", file_path);
        curl_easy_cleanup(curl);
        return -1;
    }
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    
    // 3 执行curl 
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        log_error("curl_easy_perform error: %s", curl_easy_strerror(res));
        fclose(file);
        curl_easy_cleanup(curl);
        return -1;
    }

    // 4 释放资源curl 
    curl_easy_cleanup(curl);
    fclose(file); // 重要

    // 5 返回响应体
    return 0;
}
