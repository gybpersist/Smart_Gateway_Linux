#include "app_device.h"
#include "app_mqtt.h"
#include "app_pool.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
    
    app_device_init("/home/ciqi9/桌面/Smart_Gateway_Linux/serial_file");
    app_device_start();

    sleep(10000);

    return 0;
}
