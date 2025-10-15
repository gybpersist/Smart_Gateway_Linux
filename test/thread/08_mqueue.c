#define _GNU_SOURCE         // 启用GNU扩展
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <unistd.h>
#include <pthread.h>

static const char *mq_name = "/test_queue";
static mqd_t mqd;

void *first_thread_func(void *arg)
{
    printf("first thread(%d)\n", gettid());
    sleep(1);
    // 接收消息
    char buf[256];
    ssize_t bytes_received = mq_receive(mqd, buf, sizeof(buf), NULL);
    if (bytes_received == -1)
    {
        perror("mq_receive");
    }
    printf("接收到消息: %s\n", buf);
}

void *second_thread_func(void *arg)
{
    printf("second thread(%d)\n", gettid());
    // 发送一条消息
    char *msg = "Hello, mqueue!";
    if (mq_send(mqd, msg, strlen(msg) + 1, 1) == -1)
    {
        perror("mq_send");
        return NULL;
    }
    printf("发送消息成功: %s\n", msg);
}

int main()
{
    // 设置队列属性
    struct mq_attr attr;
    attr.mq_maxmsg = 10;   // 队列最多可以包含 10 条消息
    attr.mq_msgsize = 256; // 每条消息的最大字节数为 256
    // 创建或打开消息队列
    mqd = mq_open(mq_name, O_CREAT | O_RDWR, 0644, &attr);
    if (mqd == (mqd_t)-1)
    {
        perror("mq_open");
        exit(1);
    }
    // 创建两个线程
    pthread_t pt1, pt2;
    pthread_create(&pt1, NULL, first_thread_func, NULL);
    pthread_create(&pt2, NULL, second_thread_func, NULL);
    // 等待分线程执行完毕
    pthread_join(pt1, NULL);
    pthread_join(pt2, NULL);
    // 关闭和删除消息队列
    mq_close(mqd);
    mq_unlink(mq_name);
    return 0;
}

// 使用 ls /dev/mqueue 查看队列可以查看生成的队列
