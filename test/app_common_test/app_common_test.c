#include "app_common.h"
#include "log/log.h"

int main(int argc, char *argv[])
{
    // 获取当前时间戳
    long time = app_common_getCurrentTime();
    log_info("time: %ld", time);
    return 0;
}

