#include "app_bt.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "log/log.h"
#include "app_serial.h"
/**
 * 初始化
 * 1. 将蓝牙数据的预处理方法装配给设备模块
 * 2 设置设备的蓝牙默认配置，并连接蓝牙
 */
int app_bt_init(Device *device)
{
    device->pre_write = app_bt_preWrite;
    device->post_read = app_bt_postRead;

    // 初始化串口
    app_serial_init(device);

    // 设置串口为非阻塞模式（用来测试AT指令）
    app_serial_setBlock(device, 0);
    tcflush(device->fd, TCIFLUSH);

    // 判断蓝牙是否可用
    if (app_bt_status(device) == 0)
    {
        // 修改蓝牙的波特率
        app_bt_setBraudRate(device, BT_BAUD_115200);
        // 设置蓝牙的NetID和MAddr
        app_bt_setNetId(device, "7919");
        app_bt_setMAddr(device, "0055");
        // 设置蓝牙名称
        app_bt_rename(device, "blename");
        // 重置
        app_bt_reset(device);
        // 重置完成要等待2s左右的时间
        sleep(2);
    }

    // 设置串口的波特率和蓝牙一致
    app_serial_setBraudRate(device, BR_115200);
    tcflush(device->fd, TCIFLUSH);

    // 判断蓝牙是否可用
    if (app_bt_status(device) != 0)
    {
        log_error("蓝牙模块不可用");
        return -1;
    }

    // 把串口设置为阻塞模式
    app_serial_setBlock(device, 1);
    tcflush(device->fd, TCIFLUSH);

    log_debug("蓝牙模块初始化完成");

    return 0;
}

/**
 * 写前预处理
 * 123XXabc -> 41 54 2b 4d 45 53 48 00 ff ff 11 22 33 0d 0a
 * 41 54 2b 4d 45 53 48 00-> AT+MESH\0
 * ff ff->设备id
 * 11 22 33 ->数据
 * 0d 0a->\r\n
 */
int app_bt_preWrite(char *data, int len)
{
    // 判断被处理的数据是不是完整的数据
    if (len < 6)
    {
        log_error("data长度太小,不合法");
        return -1;
    }

    // 定义存放结果的缓冲区
    int blue_len = 12 + data[2];
    char blue_data[blue_len];

    // 依次存放数据
    memcpy(blue_data, "AT+MESH", 8);
    memcpy(blue_data + 8, data + 3, 2);
    memcpy(blue_data + 10, data + 5, data[2]);
    memcpy(blue_data + 10 + data[2], "\r\n", 2);

    // 我们当前函数作用是直接把data内容改变
    memset(data, 0, len);
    memcpy(data, blue_data, blue_len);

    return blue_len;
}

/**
 *  读后预处理
 */
/**
 * 从设备读取数据后处理数据来满足网关应用的要求
 * 接收方得到数据（3 + [2]）：f1 dd 07 23 23 ff ff 41 42 43
 * f1 dd : 固定的头部
 * 07：之后数据的长度（5-16之间）
 * 23 23：对端（发送方）的 MADDR
 * ff ff: 我的 MADDR 或 ffff(群发)
 * 41 42 43：发送的数据
 * 处理后的数据格式：conn_type id_len msg_len id msg
 */
static char fix_header[] = {0xF1, 0xDD};
// 定义一个缓冲区 用来收集每次读到的可能不完整的数据
static char read_buf[1024];
static int read_len = 0;
int app_bt_postRead(char *data, int len)
{
    // 把当前读到的数据 追加到read_buf中
    memcpy(read_buf + read_len, data, len);
    read_len += len;

    // 只要缓冲区的长度还是不够8位 则直接返回 继续等待下次读
    if (read_len < 8)
    {
        return -1;
    }

    // 读到长度大于8的数据 我还要判断是否包含 f1 dd
    int i;
    for (i = 0; i < read_len - 7; i++)
    {
        if (memcmp(read_buf + i, fix_header, 2) == 0)
        {
            // 移除之前所有的废数据
            memmove(read_buf, read_buf + i, read_len - i);
            read_len -= i;

            // 判断数据长度是否足够
            if (read_len < (3 + read_buf[2]))
            {
                return -1;
            }

            // 数据整理
            memset(data, 0, len);
            data[0] = 1;
            data[1] = 2;
            data[2] = read_buf[2] - 4;
            memcpy(data + 3, read_buf + 3, 2);
            memcpy(data + 5, read_buf + 7, data[2]);

            memmove(read_buf, read_buf + read_buf[2] + 3, read_len - read_buf[2] - 3);
            read_len -= (read_buf[2] + 3);

            return data[2] + 5;
        }
    }

    memmove(read_buf, read_buf + i, read_len - i);
    read_len -= i;

    return -1;
}

/**
 * 等待蓝牙模块的确认
 */
static int wait_ack(Device *device)
{
    // 注意：需要等待一定时间，因为蓝牙模块需要一定时间来处理指令
    usleep(1000 * 50); // 50ms

    // 从设备中读取4个字节的数据
    char buf[4];
    read(device->fd, buf, 4);
    // 如果读取的数据是 "OK\r\n" 则代表成功，返回0, 否则返回-1
    if (memcmp(buf, "OK\r\n", 4) == 0)
    {
        return 0;
    }
    return -1;
}

/**
 * 测试蓝牙是否可用
 */
int app_bt_status(Device *device)
{
    // 向串口发送测试指令 也就是写入 "AT\r\n"
    write(device->fd, "AT\r\n", 4);
    // 返回是否成功/确认的结果
    return wait_ack(device);
}

/**
 * 修改蓝牙名称
 */
int app_bt_rename(Device *device, char *name)
{
    char cmd[100];
    sprintf(cmd, "AT+NAME%s\r\n", name);
    write(device->fd, cmd, strlen(cmd));

    return wait_ack(device);
}

/**
 * 设置波特率
 */
int app_bt_setBraudRate(Device *device, BTBraudRate baud_rate)
{
    char cmd[100];
    sprintf(cmd, "AT+BAUD%c\r\n", baud_rate);
    write(device->fd, cmd, strlen(cmd));

    return wait_ack(device);
}

/**
 * 重置(修改的配置才生效)
 */
int app_bt_reset(Device *device)
{
    write(device->fd, "AT+RESET\r\n", 10);

    return wait_ack(device);
}

/**
 * 设置组网 id(同一个组的多个设备组网 id 相同)
 * net_id：4位十六进制字符串
 */
int app_bt_setNetId(Device *device, char *net_id)
{
    char cmd[100];
    sprintf(cmd, "AT+NETID%s\r\n", net_id);
    write(device->fd, cmd, strlen(cmd));

    return wait_ack(device);
}

/**
 * 设置蓝牙 MAC 地址(同一个组的多个设备 MAC 地址不同)
 * maddr: 4位十六进制字符串
 */
int app_bt_setMAddr(Device *device, char *maddr)
{
    char cmd[100];
    sprintf(cmd, "AT+MADDR%s\r\n", maddr);
    write(device->fd, cmd, strlen(cmd));

    return wait_ack(device);
}