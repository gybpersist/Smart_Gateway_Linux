#ifndef __OTA_HTTP_H__
#define __OTA_HTTP_H__

#define OTA_URL_FILEINFO "http://192.168.11.57:8000/fileinfo.json"
#define OTA_URL_DOWNLOAD "http://192.168.11.57:8000/download/gateway"
#define OTA_LOCAL_FILE_PATH "/home/ciqi9/桌面/gateway.update"  // 测试使用
// #define OTA_LOCAL_FILE_PATH "/root/gateway.update"        // 正式使用

/**
 * 访问url，获取json数据
 */
char *ota_http_getJson(char *url);

/**
 * 访问url, 下载文件到指定位置
 */
int ota_http_download(char *url, char *filename);

#endif /* __OTA_HTTP_H__ */
