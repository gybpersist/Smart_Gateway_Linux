#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
    printf("父进程开始执行\n");

    pid_t pid = fork();
    if (pid > 0)
    {
        int subp_status;
        printf("父进程接着执行中... \n");
        // 等待指定的子进程结束 => 这样父进程就只会在子进程结束后才能结束，子进程就不会成为孤儿进程
        // waitpid(pid, NULL, 0);
        waitpid(pid, &subp_status, 0);
        printf("父进程 wait 结束，得到子进程执行结果状态值：%d\n", subp_status);
    }
    else if (pid == 0)
    {
        printf("子进程开始执行, 准备运行 ping 命令\n");
        char *argv[] = {"/bin/ping", "-c", "10", "www.baidu.com", NULL};
        char *envp[] = {NULL};
        execve(argv[0], argv, envp);
        _exit(EXIT_FAILURE);
    }

    printf("父进程即将执行结束\n");

    return 0;
}
