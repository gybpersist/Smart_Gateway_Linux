#define _GNU_SOURCE
#include"app_bt.h"
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include"log/log.h"
#include "app_serial.h"

/**
 * 初始化
 * 1. 将蓝牙数据的预处理方法装配给设备模块
 * 2 设置设备的蓝牙默认配置，并连接蓝牙
 */
int app_bt_init(Device *device)
{
    // 1. 将蓝牙数据的预处理方法装配给设备模块
    device->pre_write = app_bt_preWrite;
    device->post_read = app_bt_postRead;

    // 2 设置设备的蓝牙默认配置，并连接蓝牙
    return 0;
}

/**
 * 写前预处理
 */
int app_bt_preWrite(char *data, int len)
{
    // data数据长度小于6，不合法
    if (len < 6)
    {
        log_error("data长度过小,不合法");
        return -1;
    }
    // 蓝牙数据的长度
    int blue_len = 12 + data[2];
    // 蓝牙数据缓存
    char blue_data[blue_len];

    // 依次将蓝牙数据的各部分拷贝到蓝牙数据缓存中
    // AT+MESH（固定头部）
    memcpy(blue_data, "AT+MESH", 8);
    // 对端的MADDR (data中的id)
    memcpy(blue_data + 8, data + 3, 2);
    // 要发送的数据（不超过12字节）  就是msg
    memcpy(blue_data + 10, data + 5, data[2]);
    // \r\n（固定结尾）
    memcpy(blue_data + 10 + data[2], "\r\n", 2);

    // 要将data中原有的数据替换为blue_data中的数据
    memset(data, 0, len);
    memcpy(data, blue_data, blue_len);

    // 返回蓝牙数据的长度
    return blue_len;
}

// 缓冲区
static char read_buf[1024];
static int read_len = 0;
static char fix_header[] = {0xF1, 0xDD};

// 删除 read_buf 中前面指定长度数据
static void remove_data_buff(int len)
{
    memmove(read_buf, read_buf + len, read_len - len);
    read_len -= len;
}

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
int app_bt_postRead(char *data, int len)
{
    // 将 data 添加到 read_buf
    memcpy(read_buf + read_len, data, len);
    read_len += len;

    // 如果 read_len 小于 8, 说明数据不完整，直接返回
    if (read_len < 8)
    {
        return 0;
    }

    int i;
    // 遍历缓存数据
    for (i = 0; i < read_len - 7; i++)
    {
        // 如果找到 f1dd, 这一段就可能是一个完整的蓝牙数据包
        if (memcmp(read_buf + i, fix_header, 2) == 0)
        {
            // 移除左侧数据
            remove_data_buff(i);

            // 如果数据长度不够，直接返回 0
            if (read_len < read_buf[2] + 3)
            {
                return 0;
            }

            /*
            f1 dd 07 23 23 ff ff 41 42 43
            f1 dd : 固定的头部
            07：之后数据的长度（5-16之间）
            23 23：对端（发送方）的 MADDR
            ff ff: 我的 MADDR 或 ffff(群发)
            41 42 43：发送的数据
            */

            // 要生成的数据：网关中 message 的二进制数据 conn_type id_len msg_len id msg
            memset(data, 0, len);
            data[0] = 1;                       // conn_type
            data[1] = 2;                       // id_len
            data[2] = read_buf[2] - 4;         // msg_len
            memcpy(data + 3, read_buf + 3, 2); // id
            memcpy(data + 5, read_buf + 7, data[2]);

            // 从缓存移除这段数据
            remove_data_buff(read_buf[2] + 3);

            return data[2] + 5;
        }
    }

    // 移除 i 个无用数据
    remove_data_buff(i);

    return 0;
}

/**
 * 初始化蓝牙连接
 */
static int init_bt(Device *device)
{
    // 初始串口 9600 / 阻塞
    app_serial_init(device);

    // 设置串口为非阻塞模式
    app_serial_setBlock(device, 0);
    tcflush(device->fd, TCIOFLUSH);

    // 判断蓝牙可用才去设置蓝牙属性
    if (app_bt_status(device) == 0)
    {
        // 修改蓝牙波特率
        app_bt_setBaudRate(device, BR_115200);
        // 设置蓝牙的 Netid/MADDR/Name
        app_bt_rename(device, "atguigu");
        app_bt_setNetId(device, "1234");
        app_bt_setMAddr(device, "0001");
        // 重置
        app_bt_reset(device);
        sleep(2);  // 等待蓝牙复位
    }

    // 将串口的波特率修改为115200
    app_serial_setBraudRate(device, BR_115200);
    tcflush(device->fd, TCIOFLUSH);

    // 判断蓝牙是否可用，如果不可用则返回-1
    if (app_bt_status(device) != 0)
    {
        log_error("蓝牙初始化失败");
        return -1;
    }

    // 将串口改为阻塞模式
    app_serial_setBlock(device, 1);
    tcflush(device->fd, TCIOFLUSH);

    log_debug("蓝牙初始化成功");
    return 0;
}

/**
 * 初始化
 */
int app_bt_init(Device *device)
{
    // 给 device 配置由蓝牙模块实现的 post_read 和 pre_write
    device->post_read = app_bt_postRead;
    device->pre_write = app_bt_preWrite;

    // 初始化蓝牙
    init_bt(device);

    return 0;
}

/**
 * 等待蓝牙模块的确认
 */
static int wait_ack(Device *device)
{
    // 注意：需要等待一定时间，因为蓝牙模块需要一定时间来处理指令
    usleep(1000 * 50);  // 50ms

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
