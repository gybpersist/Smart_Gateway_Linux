#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THREAD_COUNT 20000

// 初始化互斥锁
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void *add_thread(void *argv)
{

    int *p = argv;

    // 累加改变数据前加锁（此时其它线程不能进入）
    pthread_mutex_lock(&counter_mutex);
    *p = *p + 1;
    // 累加改变数据后释放锁 (此时其它线程可以进入)
    pthread_mutex_unlock(&counter_mutex);

    return NULL;
}

int main()
{
    pthread_t threads[THREAD_COUNT];

    int num; // 被n个线程进行不断累加的变量

    // 启动20000个线程对num进行累加
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_create(&threads[i], NULL, add_thread, &num);
    }

    // 等待所有线程执行的结果
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // 打印累加结果
    printf("累加结果：%d\n", num);

    // 销毁锁，释放锁占用的所有内存和资源
    pthread_mutex_destroy(&counter_mutex);

    return 0;
}
