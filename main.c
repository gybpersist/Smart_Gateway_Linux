#include "app_runner.h"
#include "log/log.h"
#include <string.h>

/**
 * 应用的入口程序
 */
int main(int argc, char const *argv[])
{
    if (argc == 1)
    {
        log_error("请输入参数");
        return -1;
    }

    if (strcmp(argv[1], "app") == 0)
    {
        app_runner_run();
    }
    else
    {
        return -1;
    }

    return 0;
}
