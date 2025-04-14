#include "ethernet.h"
#include "common.h"
#include "mqtt.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "ethernet_init.h"  // Make sure this exists in your project

static const char *TAG = "ethernet";
static esp_eth_handle_t s_eth_handle = NULL;

char actual_ip[16] = {0};
uint8_t assigned_mac[6] = {0};

static void eth_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(s_eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        memcpy(assigned_mac, mac_addr, 6);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

static void got_ip_event_handler(void *arg, esp_event_base_t event_base, 
                                int32_t event_id, void *event_data)
{
    char stat_msg[50];
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;
    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "___________________");
    ESP_LOGI(TAG, "IP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "MASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "GW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "___________________");
    sprintf(actual_ip, IPSTR, IP2STR(&ip_info->ip));

    // Start MQTT after getting IP
    mqtt_app_start();
}

void ethernet_init(void)
{
    
    esp_netif_init();
    esp_event_loop_create_default();

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&netif_cfg);
   
    // Set static IP
    esp_netif_dhcpc_stop(eth_netif);
    esp_netif_ip_info_t ip_info;
    esp_netif_str_to_ip4(DEVICE_IP, &ip_info.ip);
    esp_netif_str_to_ip4(DEVICE_SUBNET, &ip_info.netmask);
    esp_netif_str_to_ip4(DEVICE_GETWAY, &ip_info.gw);
    esp_netif_set_ip_info(eth_netif, &ip_info);

    // Initialize Ethernet hardware
    uint8_t eth_port_cnt = 0;
    esp_eth_handle_t *eth_handles;
    example_eth_init(&eth_handles, &eth_port_cnt);
   
    s_eth_handle = eth_handles[0];
    free(eth_handles);

    esp_netif_attach(eth_netif, esp_eth_new_netif_glue(s_eth_handle));
   
    // Register event handlers
    esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL);

    esp_eth_start(s_eth_handle);
}

esp_eth_handle_t get_eth_handle(void)
{
    return s_eth_handle;
}