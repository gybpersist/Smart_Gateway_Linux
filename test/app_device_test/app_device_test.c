#include "app_device.h"
#include "app_mqtt.h"
#include "app_pool.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
    // 使用 echo -n -e "\x01\x02\x03\x58\x58\x61\x62\x63" >>serial_file 在终端写入数据到serial_file中
    app_device_init("/home/ciqi9/桌面/Smart_Gateway_Linux/serial_file");
    app_device_start();

    sleep(10000);

    return 0;
}
