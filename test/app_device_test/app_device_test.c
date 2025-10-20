#include "app_device.h"
#include "app_mqtt.h"
#include "app_pool.h"
#include "unistd.h"
#include "app_bt.h"

int main(int argc, char *argv[])
{
    // 使用 echo -n -e "\x01\x02\x03\x58\x58\x61\x62\x63" >>serial_file 在终端写入数据到serial_file中
    Device *device = app_device_init("/home/ciqi9/桌面/Smart_Gateway_Linux/serial_file");
    /*
    echo -n -e "\xf9\xf9\x01\x01\x33\x33" >>serial_file
    echo -n -e "\x00\x00\x00\xff\xf1\xdd" >>serial_file
    echo -n -e "\x07\x23\x23\xff\xff\x41" >>serial_file
    echo -n -e "\x42\x43\x88\x00\x99\x41" >>serial_file
    */
    // 初始化蓝牙
    app_bt_init(device);
    app_device_start();

    sleep(10000);

    return 0;
}
