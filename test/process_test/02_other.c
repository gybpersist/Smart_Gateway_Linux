#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    printf("other程序进程(%d)执行...\n", getpid());

    return 0;
}
