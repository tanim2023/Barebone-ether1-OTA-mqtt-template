#ifndef ETHERNET_H
#define ETHERNET_H

#include "esp_eth.h"

// Initialize ethernet interface
void ethernet_init(void);

// Get ethernet handle
esp_eth_handle_t get_eth_handle(void);

#endif // ETHERNET_H