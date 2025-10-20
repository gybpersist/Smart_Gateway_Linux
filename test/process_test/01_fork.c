#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    printf("父(main)进程(%d)开始执行\n", getpid());
    int val = 123;

    // 创建子进程
    pid_t pid = fork();

    if (pid > 0)  // 父进程
    {
        val++;
        printf("父进程(%d)执行, 子进程id=%d, val=%d\n", getpid(), pid, val);
    }
    else if (pid == 0)  // 子进程
    {
        val--;
        printf("子进程(%d)执行, 父进程id=%d, value=%d\n", getpid(), getppid(), val);
    }
    else  // 创建进程失败，直接失败退出
    {
        perror("创建子进程失败");
        return 1;
    }

    return 0;
}
