/**
 * @file lan8720.cpp
 * @author Eddy Beaupré (https://github.com/EddyBeaupre)
 * @brief Handle SmartConfig configuration
 * @version 1.1.0
 * @date 2021-10-03
 * 
 * @copyright Copyright 2021 Eddy Beaupré <eddy@beaupre.biz>
 *            Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 *            following conditions are met:
 * 
 *            1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 *               disclaimer.
 * 
 *            2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 *               following disclaimer in the documentation and/or other materials provided with the distribution.
 * 
 *            THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS  "AS IS"  AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *            INCLUDING,   BUT NOT LIMITED TO,  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *            DISCLAIMED.   IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *            SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL DAMAGES  (INCLUDING,  BUT NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE GOODS OR
 *            SERVICES;  LOSS OF USE, DATA, OR PROFITS;  OR BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *            WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *            OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>
#include <esp_netif.h>
//#include <esp_eth.h>
#include <esp_event.h>
#include <esp_log.h>

#include "lan8720.hpp"

#define LOGI(v, t, f, ...)             \
    if (v)                             \
    {                                  \
        ESP_LOGI(t, f, ##__VA_ARGS__); \
    }

lan8720::lan8720(int smi_mdc_gpio_num, int smi_mdio_gpio_num, int reset_gpio_num, int32_t phy_addr, bool verbose)
{
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    mac_config.smi_mdc_gpio_num = smi_mdc_gpio_num;
    mac_config.smi_mdio_gpio_num = smi_mdio_gpio_num;
    phy_config.phy_addr = phy_addr;
    phy_config.reset_gpio_num = reset_gpio_num;

    setup(mac_config, phy_config, verbose);
}

lan8720::lan8720(eth_mac_config_t mac_config, eth_phy_config_t phy_config, bool verbose)
{
    setup(mac_config, phy_config, verbose);
}

void lan8720::setup(eth_mac_config_t mac_config, eth_phy_config_t phy_config, bool verbose)
{
    verboseLoggin = verbose;

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&netif_config);

    ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));

    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));

    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eventHandler, this, &eth_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &eventHandler, this, &ip_event_instance));

    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}

void lan8720::eventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    lan8720 *Instance = (lan8720 *)event_handler_arg;
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    if (event_base == ETH_EVENT)
    {
        switch (event_id)
        {
        case ETHERNET_EVENT_CONNECTED:
            if (Instance->verboseLoggin)
            {
                uint8_t mac_addr[6] = {0};
                esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
                ESP_LOGI("lan8720::eth_event_handler", "Ethernet Link Up\nEthernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            }
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            LOGI(Instance->verboseLoggin, "lan8720::eth_event_handler", "Ethernet Link Down");
            break;
        case ETHERNET_EVENT_START:
            LOGI(Instance->verboseLoggin, "lan8720::eth_event_handler", "Ethernet Started");
            break;
        case ETHERNET_EVENT_STOP:
            LOGI(Instance->verboseLoggin, "lan8720::eth_event_handler", "Ethernet Stopped");
            break;
        default:
            break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_ETH_GOT_IP:
        {
            if (Instance->verboseLoggin)
            {
                ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
                const esp_netif_ip_info_t *ip_info = &event->ip_info;
                ESP_LOGI("lan8720::ip_event_handler", "Ethernet IP Address: " IPSTR ", Netmask: " IPSTR ", Gateway: " IPSTR, IP2STR(&ip_info->ip), IP2STR(&ip_info->netmask), IP2STR(&ip_info->gw));
            }
            break;
        }
        default:
            break;
        }
    }
}
