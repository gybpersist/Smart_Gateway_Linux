#include "log/log.h"
#include "ota_version.h"
#include <unistd.h>
#include "ota_http.h"
#include "cJSON/cJSON.h"
#include <openssl/sha.h>
#include <sys/reboot.h>
#include <string.h>

/**
 * 获取文件的SHA1哈希值（40位16进制字符串）
 * 相同文件返回相同的哈希值，可用于判断文件是否相同
 * linux命令生成：sha1sum 文件名
 * 此函数可以利用AI工具帮我们生成
 */
static char *get_file_sha(char *filepath)
{
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX sha1;
    SHA1_Init(&sha1);

    const int bufSize = 32768;
    unsigned char *buffer = (unsigned char *)malloc(bufSize);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    int bytesRead;
    while ((bytesRead = fread(buffer, 1, bufSize, file)) > 0) {
        SHA1_Update(&sha1, buffer, bytesRead);
    }

    SHA1_Final(hash, &sha1);
    fclose(file);
    free(buffer);

    char *outputBuffer = (char *)malloc(SHA_DIGEST_LENGTH * 2 + 1);
    if (!outputBuffer) {
        perror("Failed to allocate memory");
        return NULL;
    }

    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }

    return outputBuffer;
}

/**
 * 检查并更新程序
 * 1. 获取远程版本信息
 * 2. 解析版本信息json, 得到远程的各级版本号
 * 3. 比较远程和本地的版本号，如果远程版本号不大于本地版本号，则无需更新
 * 4. 如果远程版本号大于本地版本号，则下载远程程序（固件）
 * 5. 下载完成后，生成文件的SHA1哈希值，同时得到远程版本信息中的SHA1哈希值相同
 * 6. 如果这2个哈希值相同，才认可下载的新版本，否则删除它，更新失败
 * 7. 重启系统，加载新版本运行（由脚本来处理）
 */
int ota_version_checkUpdate()
{
    char *json = ota_http_getJson(OTA_URL_FILEINFO);
    if (json == NULL) {
        log_error("获取版本信息失败");
        return -1;
    }

    log_debug("获取到的版本信息: %s", json);

    // 解析版本信息
    cJSON *root = cJSON_Parse(json);
    if (root == NULL) {
        log_error("解析JSON失败");
        free(json);
        return -1;
    }

    cJSON *major_item = cJSON_GetObjectItem(root, "major");
    cJSON *minor_item = cJSON_GetObjectItem(root, "minor");
    cJSON *patch_item = cJSON_GetObjectItem(root, "patch");
    
    if (major_item == NULL || minor_item == NULL || patch_item == NULL) {
        log_error("JSON中缺少版本号字段");
        free(json);
        cJSON_Delete(root);
        return -1;
    }

    int version_major = major_item->valueint;
    int version_minor = minor_item->valueint;
    int version_patch = patch_item->valueint;
    log_debug("远程版本信息: %d.%d.%d", version_major, version_minor, version_patch);

    // 比较版本号
    if (VERSION_MAJOR > version_major ||
        (VERSION_MAJOR == version_major && VERSION_MINOR > version_minor) ||
        (VERSION_MAJOR == version_major && VERSION_MINOR == version_minor && VERSION_PATCH > version_patch)) {
        log_debug("当前版本已是最新版本");
        free(json);
        cJSON_Delete(root);
        return 0;
    }

    log_debug("当前版本不是最新版本, 需要更新");

    // 下载远程最新版本程序
    unlink(OTA_LOCAL_FILE_PATH);
    int result = ota_http_download(OTA_URL_DOWNLOAD, OTA_LOCAL_FILE_PATH);
    if (result != 0) {
        log_error("下载新版本失败");
        free(json);
        cJSON_Delete(root);
        return -1;
    }

    // 生成本地文件的验证码
    char *sha_local = get_file_sha(OTA_LOCAL_FILE_PATH);
    if (sha_local == NULL) {
        log_error("生成本地文件SHA1失败");
        free(json);
        cJSON_Delete(root);
        unlink(OTA_LOCAL_FILE_PATH);
        return -1;
    }

    // 与远程文件验证码比较，如果相同才认可当前文件
    cJSON *sha_item = cJSON_GetObjectItem(root, "sha1");
    if (sha_item == NULL || sha_item->valuestring == NULL) {
        log_error("JSON中缺少SHA1字段");
        free(json);
        free(sha_local);
        cJSON_Delete(root);
        unlink(OTA_LOCAL_FILE_PATH);
        return -1;
    }

    char *sha_remote = sha_item->valuestring;
    log_debug("本地文件sha1: %s", sha_local);
    log_debug("远程文件sha1: %s", sha_remote);

    if (strcmp(sha_local, sha_remote) == 0) {
        log_debug("下载文件验证成功");
        free(json);
        free(sha_local);
        cJSON_Delete(root);
        
        // 重启系统
        log_debug("准备重启系统以应用更新");
        reboot(RB_AUTOBOOT);
    } else {
        log_error("下载文件验证失败");
        free(json);
        free(sha_local);
        cJSON_Delete(root);
        unlink(OTA_LOCAL_FILE_PATH);
        return -1;
    }

    return 0;
}

void ota_version_printVersion()
{
    log_debug("当前版本号: %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

int ota_version_checkDailyUpdate()
{
    while (1) {
        ota_version_checkUpdate();
        log_debug("等待24小时后再次检查更新");
        sleep(60 * 60 * 24);
    }
}
