#define _GNU_SOURCE
#include "app_pool.h"
#include "log/log.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int task_func(void *arg)
{
    char *name = (char *)arg;
    log_debug("task func execute, name: %s, tid=%d", name, gettid());

    return 0;
}

int main(int argc, char *argv[])
{
    app_pool_init(6);

    for (int i = 0; i < 5; i++)
    {
        char *name = malloc(10);
        sprintf(name, "%s_%d", "atguigu", i);
        log_debug("add task arg= %s, tid=%d", name, gettid());
        app_pool_registerTask(task_func, name);
    }

    sleep(1);
    app_pool_close();

    return 0;
}
