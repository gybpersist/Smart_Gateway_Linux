#include "ota_http.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{
    char *json = ota_http_getJson(OTA_URL_FILEINFO);
    printf("json = %s\n", json);

    int result = ota_http_download(OTA_URL_DOWNLOAD, OTA_LOCAL_FILE_PATH);
    if (result == 0) {
        printf("download success\n");
    } else {
        printf("download fail\n");
    }

    return 0;
}
