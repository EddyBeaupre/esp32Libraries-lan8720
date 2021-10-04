/**
 * @file lan8720.hpp
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

#pragma once

#include <esp_netif.h>
#include <esp_event.h>

class lan8720
{
public:
    lan8720(int, int, int, int32_t, bool);
    lan8720(eth_mac_config_t, eth_phy_config_t, bool);
    ~lan8720();

private:
    esp_netif_t *eth_netif = NULL;
    esp_eth_handle_t eth_handle = NULL;
    esp_event_handler_instance_t eth_event_instance;
    esp_event_handler_instance_t ip_event_instance;
    bool verboseLoggin = false;

    void setup(eth_mac_config_t, eth_phy_config_t, bool);
    static void eventHandler(void *, esp_event_base_t, int32_t, void *);
};
