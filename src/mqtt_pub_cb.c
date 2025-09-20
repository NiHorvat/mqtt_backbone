#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <arpa/inet.h>

#include "mqtt_pub_cb.h"



void process_received_message(char *t, char *m){

    printk("t : %s, m : %s", t, m);

}