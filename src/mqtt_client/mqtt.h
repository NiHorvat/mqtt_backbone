#ifndef MQTT_H_
#define MQTT_H_

/**
 * Username and password if the broker requires username and password to log in
 * 
 * 
*/
#define MQTT_BROKER_USERNAME        CONFIG_MQTT_CLIENT_ID
#define MQTT_BROKER_PSWD            CONFIG_MQTT_BROKER_PASSWORD

#define MQTT_CONN_TIMEOUT_MS        CONFIG_MQTT_CONN_TIMEOUT_MS
#define MQTT_PUB_ACK_TIMEOUT_MS     CONFIG_MQTT_PUB_ACK_TIMEOUT_MS

#define TX_BUFFER_SIZE              CONFIG_MQTT_TX_BUFFER_SIZE
#define RX_BUFFER_SIZE              CONFIG_MQTT_RX_BUFFER_SIZE

/**
 * Is a blocking function -> it is not using any polling
*/
int mqtt_connect_to_broker(
    char* mqtt_broker_ip, 
    int mqtt_broker_port,
    const char *username,
    const char *password
    );



/**
 * @brief publish raw (char *) data to a specific topic with a specific qos
 * @param topic NULL terminated const char *
 * @param data NULL terminated const char *
 * @param qos if not 0, will block and wait for ACK from the broker
 * 
 * 
*/
int mqtt_publish_to_topic(const char *topic, const char *data, int qos);


/**
 * @brief subscribe to @param len topics in @param topic_arr 
 * @note void process_received_message(char *t, char *m) from @file mqtt_pub_cb.h will be called upon receiving a message on a subscribed topic
 * 
*/
int mqtt_subscribe_to_topic(const char **topic_arr, const uint16_t len);



/***
 * @brief Blocking, keep the connection alive and "polls" for events
 * 
 * 
*/
void mqtt_loop();


#endif /**MQTT_H_*/


