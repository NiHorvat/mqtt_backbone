#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_ip.h>

#include "wifi.h"


// Event callbacks
static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

// Semaphores
static K_SEM_DEFINE(sem_wifi, 0, 1);
static K_SEM_DEFINE(sem_ipv4, 0, 1);

// Connection status
static int wifi_connection_status = -1;


// Called when the WiFi is connected
static void on_wifi_connection_event(struct net_mgmt_event_callback *cb,
                                     uint64_t mgmt_event,
                                     struct net_if *iface)
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT) {
        wifi_connection_status = status->status;
        if (wifi_connection_status) {
            printk("Error (%d): Connection request failed\r\n", wifi_connection_status);
        } else {
            printk("Connected!\r\n");
        }
    } else if (mgmt_event == NET_EVENT_WIFI_DISCONNECT_RESULT) {
        if (status->status) {
            printk("Error (%d): Disconnection request failed\r\n", status->status);
        } else {
            printk("Disconnected\r\n");
        }
    }

    k_sem_give(&sem_wifi); 

}

// Event handler for IPv4 address acquisition
static void on_ipv4_obtained(struct net_mgmt_event_callback *cb,
                             uint64_t mgmt_event,
                             struct net_if *iface)
{
    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
        k_sem_give(&sem_ipv4);
    }
}

// Initialize the WiFi event callbacks
void wifi_init(void)
{
    net_mgmt_init_event_callback(&wifi_cb,
                                 on_wifi_connection_event,
                                 NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_add_event_callback(&wifi_cb);

    net_mgmt_init_event_callback(&ipv4_cb,
                                 on_ipv4_obtained,
                                 NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&ipv4_cb);
}

// Connect to the WiFi network (blocking)
int wifi_connect(char *ssid, char *psk)
{

    int ret;
    struct net_if *iface;
    struct wifi_connect_req_params params = {
        .ssid = (const uint8_t *)ssid,
        .ssid_length = strlen(ssid),
        .psk = (const uint8_t *)psk,
        .psk_length = strlen(psk),
        .security = WIFI_SECURITY_TYPE_PSK,
        .band = WIFI_FREQ_BAND_UNKNOWN,
        .channel = WIFI_CHANNEL_ANY,
        .mfp = WIFI_MFP_OPTIONAL
    };


    printk("%s Attempting Wifi connection\n",__func__);
    iface = net_if_get_default();
    ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params));
    if (ret < 0) {
        printk("Connection request failed: %d\r\n", ret);
        return ret;
    }

    k_sem_take(&sem_wifi, K_SECONDS(20));
    
    return wifi_connection_status;
}

// Wait for IP address (blocking)
void wifi_wait_for_ip_addr(void)
{
    struct net_if *iface = net_if_get_default();

    // Clear any pending semaphore signals
    while (k_sem_take(&sem_ipv4, K_NO_WAIT) == 0) {
        // Discard any previous signals
    }

    // Check if we already have an IPv4 address
    if (net_if_ipv4_get_global_addr(iface, NET_ADDR_PREFERRED)) {
        printk("IPv4 address already assigned\r\n");
        return;
    }

    // Wait for new address assignment
    k_sem_take(&sem_ipv4, K_FOREVER);
}

// Disconnect from the WiFi network
int wifi_disconnect(void)
{
    int ret;
    struct net_if *iface = net_if_get_default();
    ret = net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0);
    return ret;
}