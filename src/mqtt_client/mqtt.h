#ifndef MQTT_H_
#define MQTT_H_

#define MQTT_BROKER_USERNAME    "test"
#define MQTT_BROKER_PSWD        "1234"

#define MQTT_CONN_TIMEOUT_MS 5000
#define MQTT_PUB_ACK_TIMEOUT_MS 5000

#define TX_BUFFER_SIZE 256
#define RX_BUFFER_SIZE 256

/**
 * Is a blocking function -> it is not using any polling
*/
int mqtt_connect_to_broker(
    const char* mqtt_broker_ip, 
    int mqtt_broker_port,
    const char *username,
    const char *password
    );

int mqtt_publish_to_topic(const char *topic, const char *data);

int mqtt_subscribe_to_topic(const char **topic_arr, const uint16_t len);

void mqtt_loop();


#endif /**MQTT_H_*/


