#define _GNU_SOURCE
#include "app_serial.h"
#include "log/log.h"

int app_serial_init(Device *device)
{
    // 设置原始模式
    app_serial_setRaw(device);
    // 设备波特率
    app_serial_setBraudRate(device, BR_9600); // 初始设置低的，后面修改为115200
    // 设置校验位
    app_serial_setParity(device, Parity_NONE);
    // 设置停止位
    app_serial_setStopBits(device, SB_1);
    // 刷新串口，让设置生效
    int result = tcflush(device->fd, TCIOFLUSH);
    if (result != 0)
    {
        log_error("串口初始化设置失败");
        return -1;
    }

    log_debug("串口初始化设置成功");
    return 0;
}

int app_serial_setBraudRate(Device *device, BraudRate rate)
{
    // 获取串口属性
    struct termios attr;
    if (tcgetattr(device->fd, &attr) != 0)
    {
        log_error("tcgetattr error");
        return -1;
    }

    // 修改属性（波特率）
    cfsetspeed(&attr, rate);

    // 设置串口属性(指定暂不生效，调用刷新才生效)
    if (tcsetattr(device->fd, TCSAFLUSH, &attr) != 0)
    {
        log_error("tcsetattr error");
        return -1;
    }

    return 0;
}

int app_serial_setParity(Device *device, Parity parity)
{
    // 获取串口属性
    struct termios attr;
    if (tcgetattr(device->fd, &attr) != 0)
    {
        log_error("tcgetattr error");
        return -1;
    }

    // 修改属性（校验位）
    attr.c_cflag &= ~(PARENB | PARODD); // 清除校验位
    attr.c_cflag |= parity;             // 指定校验位

    // 设置串口属性(指定暂不生效，调用刷新才生效)
    if (tcsetattr(device->fd, TCSAFLUSH, &attr) != 0)
    {
        log_error("tcsetattr error");
        return -1;
    }

    return 0;
}

int app_serial_setStopBits(Device *device, StopBits stop_bits)
{
    // 获取串口属性
    struct termios attr;
    if (tcgetattr(device->fd, &attr) != 0)
    {
        log_error("tcgetattr error");
        return -1;
    }

    // 修改属性（停止位）
    attr.c_cflag &= ~CSTOPB;    // 清除停止位
    attr.c_cflag |= stop_bits;  // 设置停止位

    // 设置串口属性(指定暂不生效，调用刷新才生效)
    if (tcsetattr(device->fd, TCSAFLUSH, &attr) != 0)
    {
        log_error("tcsetattr error");
        return -1;
    }

    return 0;
}

int app_serial_setBlock(Device *device, int is_block)
{
    // 获取串口属性
    struct termios attr;
    if (tcgetattr(device->fd, &attr) != 0)
    {
        log_error("tcgetattr error");
        return -1;
    }

    // 修改属性（是否阻塞）
    if (is_block == 1)
    {
        attr.c_cc[VMIN] = 1;   // 至少读取一个字节
        attr.c_cc[VTIME] = 0;  // 没有数据会一直等待
    }
    else
    {
        attr.c_cc[VMIN] = 0;   // 最少读取0个字节
        attr.c_cc[VTIME] = 2;  // 没有数据会等待2*100ms
    }

    // 设置串口属性(指定暂不生效，调用刷新才生效)
    if (tcsetattr(device->fd, TCSAFLUSH, &attr) != 0)
    {
        log_error("tcsetattr error");
        return -1;
    }

    return 0;
}

int app_serial_setRaw(Device *device)
{
    // 获取串口属性
    struct termios attr;
    if (tcgetattr(device->fd, &attr) != 0)
    {
        log_error("tcgetattr error");
        return -1;
    }

    // 修改属性（原始模式）
    cfmakeraw(&attr);

    // 设置串口属性(指定暂不生效，调用刷新才生效)
    if (tcsetattr(device->fd, TCSAFLUSH, &attr) != 0)
    {
        log_error("tcsetattr error");
        return -1;
    }

    return 0;
}
