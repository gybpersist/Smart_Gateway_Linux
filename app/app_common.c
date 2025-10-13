#include "app_common.h"
#include <stdlib.h>   // NULL
#include <sys/time.h> // timeval / gettimeofday()

/**
* @brief获取自1970年开始的时间戳，以ms为单位
* 
* @return long 时间戳
*/
long app_common_getCurrentTime(void)
{
    // 获取当前时间，单位为秒和微秒
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // tv_sec: 秒  tv_usec: 微秒
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
