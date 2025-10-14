#include "MQTTClient.h"
#include "log/log.h"
#include <string.h>
#include "app_mqtt.h"

static MQTTClient client;
static MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
static MQTTClient_message pubmsg = MQTTClient_message_initializer;

// 定义接收处理消息数据的回调函数指针
static int (*recv_callback)(char *json) = NULL;

// 发送消息完成的回调函数
static void delivered(void *context, MQTTClient_deliveryToken dt)
{
    log_debug("发送消息完成");
}

// 接收到远程发送过来的消息
// 此函数比较特别：返回1代表接收处理函数成功了，返回0代表接收处理函数失败了
static int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int result = 0;
    if (recv_callback != NULL)
    {
        result = recv_callback(message->payload) == 0 ? 1 : 0;
    }

    // 释放内存
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    return result;
}

// 连接断开时的回调函数
static void connlost(void *context, char *cause)
{
    log_error("连接断开，原因：%s", cause);
}

/**
 * @brief初始化mqtt客户端
 */
int app_mqtt_init()
{
    int result;
    // 创建mqtt客户端

    if ((result = MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        log_error("MQTTClient_create failed, return code %d", result);
        return -1;
    }
    // 设置回调函数
    if ((result = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        log_error("MQTTClient_setCallbacks failed, return code %d", result);
        MQTTClient_destroy(&client);
        return -1;
    }
    // 连接mqtt服务器
    if ((result = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        log_error("MQTTClient_connect failed, return code %d", result);
        MQTTClient_destroy(&client);
        return -1;
    }
    // 订阅主题（PULL）
    if ((result = MQTTClient_subscribe(client, TOPIC_PULL, QOS)) != MQTTCLIENT_SUCCESS)
    {
        log_error("MQTTClient_subscribe failed, return code %d", result);
        MQTTClient_disconnect(client, TIMEOUT);
        MQTTClient_destroy(&client);

        return -1;
    }

    log_debug("MQTT客户端初始化成功");

    return 0;
}

/**
 * @brief关闭mqtt客户端
 */
void app_mqtt_close()
{
    MQTTClient_unsubscribe(client, TOPIC_PULL);
    MQTTClient_disconnect(client, TIMEOUT);
    MQTTClient_destroy(&client);
}

/**
 * @brief发送消息
 */
int app_mqtt_send(char *json)
{
    pubmsg.payload = json;
    pubmsg.payloadlen = strlen(json);
    pubmsg.qos = QOS;
    if (MQTTClient_publishMessage(client, TOPIC_PUSH, &pubmsg, NULL) != MQTTCLIENT_SUCCESS)
    {
        log_error("MQTTClient_publishMessage failed");
        return -1;
    }
    log_debug("向远程发送消息：%s", json);
    return 0;
}
/**
 * @brief注册接收处理接收到的消息的回调函数
 */
void app_mqtt_registerRecvCallback(int (*callback)(char *json))
{
    recv_callback = callback;
}
