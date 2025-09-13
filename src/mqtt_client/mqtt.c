#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <arpa/inet.h>

#include "mqtt.h"
#include "mqtt_interface.h"

#define MQTT_CLIENT_ID          "board_1"



static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static struct mqtt_client client_ctx;
static struct sockaddr_storage broker;


static bool connected = false;
static bool published = false;





/**
 * Internal event handler for mqtt callback
 * after each event function can be triggered
*/
static void mqtt_evt_handler(struct mqtt_client *client,
                      const struct mqtt_evt *evt)
{
    switch (evt->type) {
    case MQTT_EVT_CONNACK:
        printk("%s Connected to the MQTT broker\n", __func__);
        connected = true;
        break;
    case MQTT_EVT_DISCONNECT:
        printk("%s Disconnected from MQTT broker\n", __func__);
        connected = false;
        break;
    case MQTT_EVT_PUBACK:
        printk("%s Publish acknowledged (QoS 1)\n", __func__);
        published = true;
        break;
    case MQTT_EVT_PUBREC:
        printk("%s Publish received confirmation (QoS 2)\n", __func__);
        break;
    case MQTT_EVT_PUBREL:
        printk("%s Publish release received (QoS 2)\n", __func__);
        break;
    case MQTT_EVT_PUBCOMP:
        printk("%s Publish complete (QoS 2)\n", __func__);
        break;
    case MQTT_EVT_SUBACK:
        printk("%s Subscribe acknowledged\n", __func__);
        break;
    case MQTT_EVT_PINGRESP:
        printk("%s Ping response received\n", __func__);
        break;
    case MQTT_EVT_PUBLISH:
    {
        // Read and process the message immediately
        uint8_t payload_buf[evt->param.publish.message.payload.len + 1];
        int ret = mqtt_read_publish_payload_blocking(
            client, 
            payload_buf, 
            evt->param.publish.message.payload.len
        );
        
        if (ret == evt->param.publish.message.payload.len) {
            payload_buf[evt->param.publish.message.payload.len] = '\0';
            
            char topic_buf[evt->param.publish.message.topic.topic.size + 1];
            memcpy(topic_buf, 
                evt->param.publish.message.topic.topic.utf8,
                evt->param.publish.message.topic.topic.size);
            topic_buf[evt->param.publish.message.topic.topic.size] = '\0';
            
            printk("Received on topic: %s\n", topic_buf);
            printk("Payload: %s\n", payload_buf);
            
            process_received_message(topic_buf, payload_buf);
        } else {
            printk("%s Failed to read payload: %d\n", __func__, ret);
        }
        break;
    }

    default:
        printk("%s Event type: %d\n", __func__, evt->type);
        break;
    }
}



int mqtt_connect_to_broker(
    const char* mqtt_broker_ip, 
    int mqtt_broker_port,
    const char *username,
    const char *password
    )
{

    uint64_t start_time;
    struct sockaddr_in *broker4 = (struct sockaddr_in *)&broker;

    
    broker4->sin_family = AF_INET;
    broker4->sin_port = htons(mqtt_broker_port);

    static const struct in_addr mqtt_ip_addr = {.s4_addr = {192,168,18,85}};
    broker4->sin_addr = mqtt_ip_addr;

    // net_addr_pton(AF_INET, mqtt_broker_ip, &mqtt_broker_ip_item);
    


    /**
     * Must be executed before any of the client paramaters are set
    */
    mqtt_client_init(&client_ctx);

    client_ctx.broker = &broker;
    client_ctx.evt_cb = mqtt_evt_handler;
    client_ctx.client_id.utf8 = (uint8_t *)MQTT_CLIENT_ID;
    client_ctx.client_id.size = strlen(MQTT_CLIENT_ID);

    client_ctx.protocol_version = MQTT_VERSION_3_1_1;
    client_ctx.transport.type = MQTT_TRANSPORT_NON_SECURE;

    // MQTT buffers configuration
    client_ctx.rx_buf = rx_buffer;
    client_ctx.rx_buf_size = sizeof(rx_buffer);
    client_ctx.tx_buf = tx_buffer;
    client_ctx.tx_buf_size = sizeof(tx_buffer);

    client_ctx.user_name = NULL;
    client_ctx.password = NULL;
    
    client_ctx.clean_session = true;

    printk("%s Starting to connect..\n", __func__);
        
        while(mqtt_connect(&client_ctx)){
            printk("Failed to connect to MQTT broker... trying again\n");
            k_sleep(K_SECONDS(5));
        }
    

    start_time = k_uptime_get();
    
    /**
     * This is stupid...
     * Must use polling but i don't know how :(
    */
    while (k_uptime_get() - start_time < MQTT_CONN_TIMEOUT_MS && !connected) {
        mqtt_input(&client_ctx);
        k_sleep(K_MSEC(100));
    }


    if (!connected) {
        printk("%s Connection timeout\n", __func__);
        mqtt_abort(&client_ctx);
        return -ETIMEDOUT;
    }

    printk("%s Successfully connected to MQTT broker\n", __func__);

    return 0;
}

int mqtt_publish_to_topic(const char *topic, const char *data){

    const int topic_len = strlen(topic);
    const int data_len = strlen(data);
    int ret;


    const struct mqtt_utf8 topic_data = {
        .utf8 = (const uint8_t *)topic,
        .size = topic_len,
    };

    const struct mqtt_topic topic_item = {
        .qos = MQTT_QOS_1_AT_LEAST_ONCE,
        .topic = topic_data,
    };


    const struct mqtt_binstr payload_item = {
        .data = (uint8_t *)data,
        .len = data_len
    };

    
    const struct mqtt_publish_message publish_msg_item = {
        .topic = topic_item,
        .payload = payload_item,
    };


    const struct mqtt_publish_param publish_param = {
        .message = publish_msg_item,
        .message_id = 5,
        .dup_flag = 1,
        .retain_flag = 1  
    };


    ret = mqtt_publish(&client_ctx, &publish_param);
    if(ret){
        printk("%s MQTT publish failed (err %d)\n",__func__, ret);
        return ret;
    }



    int64_t start_time = k_uptime_get();
    while (k_uptime_get() - start_time < MQTT_PUB_ACK_TIMEOUT_MS && !published) {
        mqtt_input(&client_ctx);
        k_sleep(K_MSEC(100));
    }

    if(!published){
        printk("%s MQTT failed to publish (topic: %s) (data: %s)\n",__func__, topic, data);
        return -1;
    }

    return 0;


}


int mqtt_subscribe_to_topic(const char **topic_arr, const uint16_t len){

    struct mqtt_topic list[len];


    for(int i = 0; i < len; i++){

        struct mqtt_utf8 topic = {
            .size = strlen(topic_arr[i]),
            .utf8 = (const uint8_t *)topic_arr[i],
        };

        list[i].qos = 0;
        list[i].topic = topic;

    }


    const struct mqtt_subscription_list params = {
        .list = list,
        .list_count = len,
        .message_id = 5,
    };

    int ret = mqtt_subscribe(&client_ctx, &params);
    if(ret){
        return ret;
    }

    return 0;

}



void mqtt_loop(){
    while(1){
        mqtt_live(&client_ctx);
        mqtt_input(&client_ctx);
        k_sleep(K_MSEC(100));
    }
}