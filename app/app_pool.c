#include "app_pool.h"
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
mqd_t mq;
int thread_num;
pthread_t *threads;

static void *thread_func(void *arg)
{
    Task task;
    while (1)
    {
        mq_receive(mq, (char *)&task, sizeof(Task), NULL);
        task.func(task.arg);
    }
}

/**
 * @brief初始化线程池(同时创建消息队列)
 * @paramsize线程池大小
 * @return int 0:成功 -1:失败
 */
int app_pool_init(int size)
{
    // 创建消息队列
    struct mq_attr attr;
    attr.mq_maxmsg = 10; // 最大消息数
    attr.mq_msgsize = sizeof(Task);
    if ((mq = mq_open("/task_queue", O_CREAT | O_RDWR, 0644, &attr)) == -1)
    {
        perror("mq_open");
        return -1;
    }

    // 创建线程池
    thread_num = size;
    threads = malloc(sizeof(pthread_t) * size);

    for (int i = 0; i < size; i++)
    {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }

    return 0;
}

/**
 * @brief关闭线程池
 * @return int 0:成功 -1:失败
 */
int app_pool_close()
{
    for (int i = 0; i < thread_num; i++)
    {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }

    free(threads);
    mq_close(mq);
    mq_unlink("/task_queue");
    return 0;
}

/**
 * @brief向线程池中添加任务
 * @paramfunc任务函数
 * @paramargv任务参数
 */
int app_pool_registerTask(int (*func)(void *), void *argv)
{
    Task task = {
        .func = func,
        .arg = argv};

    return mq_send(mq, (char *)&task, sizeof(Task), 0);
}