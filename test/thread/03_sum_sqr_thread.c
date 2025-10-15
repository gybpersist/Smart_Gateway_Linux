#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

int calc_sqr(int a)
{
    sleep(1);
    return a * a;
}

// 线程函数：求指定数值的平方并返回给主线程
void *calc_sqr_thread_fun(void *arg)
{

    int num = *(int *)arg;
    int *result = malloc(sizeof(int));
    *result = num * num;

    sleep(1);
    return result;
    // pthread_exit(result);
}

// 对一个数组中的数分别求平方然后再汇总求和
int main(int argc, char const *argv[])
{
    int items[5] = {1, 2, 3, 4, 5};
    int sqrs[5];

    struct timeval start_t, end_t;
    gettimeofday(&start_t, NULL);

    // 启动5个线程分别计算items中每个数的平方
    pthread_t pts[5];
    for (int i= 0; i < 5; i++)
    {
        pthread_create(&pts[i], NULL, calc_sqr_thread_fun, &items[i]);
    }

    // 等待所有线程执行完毕，将每个线程的返回值保存到sqrs数组中
    for (int i = 0; i < 5; i++)
    {
        void *result;
        pthread_join(pts[i], &result);
        sqrs[i] = *(int *)result;
    }

    gettimeofday(&end_t, NULL);
    // 输出计算耗时
    printf("time = %ld秒\n", end_t.tv_sec - start_t.tv_sec);

    // 累加所有的平方值
    int sum = 0;
    for (int i = 0; i < 5; i++)
    {
        sum += sqrs[i];
    }
    printf("sum = %d\n", sum);

    return 0;
}
