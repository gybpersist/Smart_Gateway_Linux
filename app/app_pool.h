#ifndef __APP_POOL_H__
#define __APP_POOL_H__

// 任务结构体
typedef struct
{
    int (*func)(void *); // 任务函数
    void *arg;           // 任务参数
} Task;

/**
 * @brief初始化线程池(同时创建消息队列)
 * @paramsize线程池大小
 * @return int 0:成功 -1:失败
 */
int app_pool_init(int size);

/**
 * @brief关闭线程池
 * @return int 0:成功 -1:失败
 */
int app_pool_close();

/**
 * @brief向线程池中添加任务
 * @paramfunc任务函数
 * @paramargv任务参数
 */
int app_pool_registerTask(int (*func)(void *), void *argv);

#endif /* __APP_POOL_H__ */
