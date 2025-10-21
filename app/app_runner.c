#include "app_device.h"
#include "unistd.h"
#include "stdlib.h"
#include "app_bt.h"
#include "signal.h"
#include "app_runner.h"

static int is_running = 1;

static void runner_exit(int sig)
{
    is_running = 0;
}

int app_runner_run()
{
    // 注册终止信号 //ctrl+c kill
    signal(SIGTERM, runner_exit); // kill
    signal(SIGINT, runner_exit);  // ctrl+c

    // 初始化设备, 内部初始化线程池和mqtt
    Device *device = app_device_init(DEVICE_FILE);

    // 初始化蓝牙，内部初始化串口
    app_bt_init(device);

    // 启动设备
    app_device_start();

    // 只要运行标识为真，一直运行着
    while (is_running)
    {
        sleep(1);
    }

    // 释放资源
    app_device_destroy();

    return 0;
}