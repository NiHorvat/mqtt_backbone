# MQTT BACKBONE

## Dependancies 
- ZephyrRTOS version
```
VERSION_MAJOR = 4
VERSION_MINOR = 2
PATCHLEVEL = 99
VERSION_TWEAK = 0
EXTRAVERSION =
```


## Overview
- Contains a **`MQTT_BACKBONE`** application used to connect to a broker, subscribe to a topic, and publish to a topic


## Usage

- To puslish to a topic use the function **`int mqtt_publish_to_topic(const char *topic, const char *data, int qos);`** from **`mqtt_client/mqtt.h`**
    - If **`qos > 0 `** the function will block the thread execution and wait for the ACK to arrive
- To subscribe to a topics use the function **`int mqtt_subscribe_to_topic(const char **topic_arr, const uint16_t len);`** from **`mqtt_client/mqtt.h`**
    -The function  **`void process_received_message(char *t, char *m);`** from **`mqtt_pub_cb.h`**, use this file to specify additional logic if required 



