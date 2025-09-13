#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <arpa/inet.h>

#include "wifi.h"
#include "mqtt.h"
#include "led.h"

// WiFi settings
#define WIFI_SSID "MagicBE97"
#define WIFI_PSK "HWTC34AF299D"

#define MQTT_BROKER_IP "192.168.18.85"
#define MQTT_BROKER_PORT        1883

#define WIFI_MAX_CONN_ATEMPTS 6 


const char* topic_arr[] = {"commands/b1"};
const uint16_t topics_len = ARRAY_SIZE(topic_arr);




int main(void)
{
    int ret;

    ret = init_indicator_led();
    if(ret){
        printk("%s Failed to set indicator LED (err %d)\n",__func__, ret);
        return 0;
    }


    printk("%s Connect to wifi\n", __func__);

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
        return 0;
    }


    ret = mqtt_subscribe_to_topic(topic_arr, topics_len);
    if(ret){
        printk("%s MQTT subscribe failed (err %d)\n",__func__, ret);
        return 0;
    }
    printk("subsribed\n");


    mqtt_loop();

    printk("end\n");

    return 0;

}   

