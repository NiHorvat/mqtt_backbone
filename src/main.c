#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <arpa/inet.h>


#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>

#include "wifi.h"
#include "mqtt.h"
#include "led.h"
#include "mqtt_pub_cb.h"

// WiFi settings
#define WIFI_SSID "MagicBE97"
#define WIFI_PSK "HWTC34AF299D"

#define MQTT_BROKER_IP "192.168.18.85"
#define MQTT_BROKER_PORT        1883
#define WIFI_MAX_CONN_ATEMPTS 6 

#define MAX_RANG      520    // Max measurement value in cm
#define ADC_SOLUTION  1023.0 // ADC resolution (10-bit like Arduino)

const char* topic_arr[] = {"commands/b1"};
const uint16_t topics_len = ARRAY_SIZE(topic_arr);


const char topic_dist[] = "distance";


K_THREAD_STACK_DEFINE(network_thread_stack, 2048);
struct k_thread network_thread;



void network_thread_func(void *arg1, void *arg2, void *arg3){

    int ret;
    // Initialize WiFi
    wifi_init();

    // Connect to the WiFi network (blocking)
    while(wifi_connect(WIFI_SSID, WIFI_PSK)){
        printk("%s Failed to connect... trying again\n",__func__);
        k_sleep(K_SECONDS(5));
    }

    // Wait to receive an IP address (blocking)
    wifi_wait_for_ip_addr();

    ret = mqtt_connect_to_broker(MQTT_BROKER_IP, MQTT_BROKER_PORT, NULL, NULL);
    if (ret != 0) {
        printk("%s MQTT failed: %d\n",__func__, ret);
        return;
    }


    ret = mqtt_subscribe_to_topic(topic_arr, topics_len);
    if(ret){
        printk("%s Failed to subscribe to a topic", __func__);
        return;
    }

    mqtt_loop();


}




int main(void)
{


    k_thread_create(&network_thread, network_thread_stack,
                   K_THREAD_STACK_SIZEOF(network_thread_stack),
                   network_thread_func,
                   NULL, NULL, NULL,
                   K_PRIO_COOP(7), 0, K_NO_WAIT);




    // Main thread is now free to process work items
    while (1) {
        k_sleep(K_FOREVER);
    }
    printk("end\n");

    return 0;

}   

