#ifndef __APP_MQTT_H__
#define __APP_MQTT_H__

// #define ADDRESS "tcp://192.168.11.57:1883"
#define ADDRESS "ws://192.168.11.57:1883"
#define CLIENTID "b253ba38-daf6-4b37-984f-5d8fdc6a1cfa"
#define TOPIC_PULL "pull"
#define TOPIC_PUSH "push"
#define PAYLOAD "Hello World!"
#define QOS 1
#define TIMEOUT 10000L

/**
 * @brief初始化mqtt客户端
 */
int app_mqtt_init();

/**
 * @brief关闭mqtt客户端
 */
void app_mqtt_close();

/**
 * @brief发送消息
 */
int app_mqtt_send(char *json);

/**
 * @brief注册接收处理接收到的消息的回调函数
 */
void app_mqtt_registerRecvCallback(int (*callback)(char *json));

#endif /* __APP_MQTT_H__ */
