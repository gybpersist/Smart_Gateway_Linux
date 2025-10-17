#include "app_pool.h"
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

static char *MQ_NAME = "/app_pool_mq";
static mqd_t mq;
static pthread_t *threads;
static int thread_num;
/**
 * @brief线程函数，不断循环接收任务并执行,如果没有任务则阻塞等待
 */
static void *thread_func(void *arg)
{
    Task task;
    // 不断循环接收任务并执行,如果没有任务则阻塞等待
    while (1)
    {
        mq_receive(mq, (char *)&task, sizeof(Task), NULL);
        task.func(task.arg);
    }
}

int app_pool_init(int size)
{
    // 初始化消息队列
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Task);
    if ((mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0666, &attr)) == -1)
    {
        perror("mq_open");
        return -1;
    }

    // 初始化线程池
    thread_num = size;
    threads = (pthread_t *)malloc(sizeof(pthread_t) * size);

    for (int i = 0; i < size; i++)
    {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }

    return 0;
}

int app_pool_close()
{
    for (int i = 0; i < thread_num; i++)
    {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }
    free(threads);

    mq_close(mq);
    mq_unlink(MQ_NAME);

    return 0;
}

int app_pool_registerTask(int (*func)(void *), void *argv)
{
    Task task = {
        .func = func,
        .arg = argv};
    return mq_send(mq, (char *)&task, sizeof(Task), 0);
}
