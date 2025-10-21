#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

static int is_running = 1;

void use_resource() {
    printf("执行很多使用资源的操作。。。\n");
}

void free_resource() {
    printf("释放资源。。。。\n");
}

void handler(int sig) {
    if (sig == SIGINT)
    {
        printf("收到 SIGINT 信号，终止进程\n");
        // exit(EXIT_FAILURE);
        is_running = 0;
    }
    else if (sig == SIGTERM)
    {
        printf("收到 SIGTERM 信号，终止进程\n");
        is_running = 0;
    }
}

int main(int argc, char const *argv[])
{
    // 注册信号处理函数 SIGINT: Ctrl+C  SIGTERM: kill 进程
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    // 模拟使用资源
    use_resource();

    // 模拟不断运行
    while (is_running)
    {
        sleep(1);
        printf("还在工作中。。。\n");
    }

    // 结束前释放资源
    free_resource();

    return 0;
}
