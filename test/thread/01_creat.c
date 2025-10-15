#define _GNU_SOURCE
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

// first线程函数
void *first_thread_func(void *arg)
{
    printf("first thread(%d)\n", gettid());
}

// second线程函数
void *second_thread_func(void *arg)
{
    printf("second thread(%d)\n", gettid());
}

int main(int argc, char const *argv[])
{
    printf("main thread(%d) start\n", gettid());

    // 创建2个分线程
    pthread_t pt1, pt2;
    pthread_create(&pt1, NULL, first_thread_func, NULL);
    pthread_create(&pt2, NULL, second_thread_func, NULL);

    // 等待分线程执行完毕
    pthread_join(pt1, NULL);
    pthread_join(pt2, NULL);

    printf("main thread(%d) end \n", gettid());
    return 0;
}
