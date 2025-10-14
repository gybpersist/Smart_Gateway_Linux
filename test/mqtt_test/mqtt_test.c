#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"

// #define ADDRESS "tcp://192.168.0.10:1883"
#define ADDRESS "tcp://broker.emqx.io:1883"
#define CLIENTID "b253ba38-daf6-4b37-984f-5d8fdc6a1cfa"
#define TOPIC_PULL "pull"
#define TOPIC_PUSH "push"
#define PAYLOAD "Hello World!"
#define QOS 1
#define TIMEOUT 10000L

static MQTTClient_message pubmsg = MQTTClient_message_initializer;

//发送消息的回调函数
void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
}

//收到远程数据的回调函数
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char *)message->payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

/**
 * @brief 处理连接丢失的回调函数
 * @param context 上下文指针，可用于传递用户自定义数据
 * @param cause 连接丢失的原因字符串
 */
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");  // 打印连接丢失的提示信息
    printf("     cause: %s\n", cause);  // 打印连接丢失的具体原因
}

int main(int argc, char *argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    //创建MQTT客户端
    if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID,
                                MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto exit;
    }

    //设置回调函数
    if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }

    //连接当前服务器
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }

    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n",
           TOPIC_PULL, CLIENTID, QOS);

    //订阅主题
    if ((rc = MQTTClient_subscribe(client, TOPIC_PULL, QOS)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to subscribe, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }
    else
    {
        int ch;
        //订阅成功 持续接收消息
        do
        {
            ch = getchar();
        } while (ch != 'Q' && ch != 'q');

        //如果按下Q/q键，则取消订阅
        if ((rc = MQTTClient_unsubscribe(client, TOPIC_PULL)) != MQTTCLIENT_SUCCESS)
        {
            printf("Failed to unsubscribe, return code %d\n", rc);
            rc = EXIT_FAILURE;
        }
    }


    //发布消息
    pubmsg.payload = PAYLOAD;
    pubmsg.payloadlen = (int)strlen(PAYLOAD);
    pubmsg.qos = QOS;
    if ((rc = MQTTClient_publishMessage(client, TOPIC_PUSH, &pubmsg, NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to publish message, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }

    //断开连接
    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to disconnect, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }
    return rc;

destroy_exit:
    MQTTClient_destroy(&client);
exit:
    return rc;
}
