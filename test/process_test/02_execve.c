#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main()
{
    printf("父(main)进程(%d)开始执行\n", getpid());

    // 创建子进程
    pid_t pid = fork();

    if (pid > 0)  // 父进程
    {
        printf("父进程(%d)执行\n", getpid());
    }
    else if (pid == 0)  // 子进程
    {
        printf("子进程(%d)执行\n", getpid());

        // 包含执行程序路径，程序名称的字符串数组
        char *argv[] = {"/home/ciqi9/桌面/Smart_Gateway_Linux/test/process_test/other", NULL};
        // 用默认的 path 环境变量
        char *envp[] = {NULL};
        // 在子进程上通过 execve() 执行其它程序, 目标程序就会在当前子进程上执行
        int result = execve(argv[0], argv, envp);
        if (result == -1)
        {
            perror("execve 执行程序失败");
            _exit(EXIT_FAILURE);
        }
    }
    else  // 创建进程失败，直接失败退出
    {
        perror("创建子进程失败");
        return 1;
    }

    return 0;
}
