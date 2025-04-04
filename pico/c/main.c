/**
 * SVI-328 PicoROM
 * 
 * Copyright (c) 2025 Markus Rautopuro
 * 
 * Works only with Raspberry Pico 2 W.
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/flash.h"
#include "wifi.h"

#include "svi-328-cartridge.h"
#include "log.h"

void error(int numblink) {
    while (1) {
        for (int i = 0; i < numblink; i++) {
            pico_set_led(true);
            sleep_ms(600);
            pico_set_led(false);
            sleep_ms(500);
        }
        sleep_ms(2000);
    }
}

#define WIFI_CONFIG_SIZE 68
#define WIFI_CONFIG_OFFSET 0x3FFFBC
#define WIFI_MAGIC_1 0xDEAD
#define WIFI_MAGIC_2 0xBEEF

extern const uint8_t __wifi_config[WIFI_CONFIG_SIZE];

int fetch_wifi_credentials(void) {
    uint16_t magic1 = (__wifi_config[0] << 8) | __wifi_config[1];
    uint16_t magic2 = (__wifi_config[2] << 8) | __wifi_config[3];

    if (magic1 != WIFI_MAGIC_1 || magic2 != WIFI_MAGIC_2) {
        log_message("No stored WiFi credentials found");
        return PICO_ERROR_GENERIC;
    }

    memcpy((void *)SSID, &__wifi_config[4], SSID_MAX_LENGTH);
    SSID[SSID_MAX_LENGTH] = '\0';

    memcpy((void *)Password, &__wifi_config[4 + SSID_MAX_LENGTH], PASSWORD_MAX_LENGTH);
    Password[PASSWORD_MAX_LENGTH] = '\0';

    log_message("Stored WiFi SSID found: [%s]", SSID);
    log_message("Stored WiFi Password found: [***] (length %zu)", strlen((void *)Password));

    return PICO_OK;
}

int store_wifi_credentials(const char *ssid, const char *password) {
    uint8_t buffer[WIFI_CONFIG_SIZE] = {0};
    buffer[0] = (WIFI_MAGIC_1 >> 8) & 0xFF;
    buffer[1] = WIFI_MAGIC_1 & 0xFF;
    buffer[2] = (WIFI_MAGIC_2 >> 8) & 0xFF;
    buffer[3] = WIFI_MAGIC_2 & 0xFF;

    strncpy((char *)&buffer[4], ssid, SSID_MAX_LENGTH);
    strncpy((char *)&buffer[4 + SSID_MAX_LENGTH], password, PASSWORD_MAX_LENGTH);

    uint32_t flash_target_offset = (XIP_BASE + WIFI_CONFIG_OFFSET);
    uint32_t aligned_sector_offset = flash_target_offset & ~(FLASH_SECTOR_SIZE - 1);

    uint8_t sector_buf[FLASH_SECTOR_SIZE];
    memcpy(sector_buf, (const void *)aligned_sector_offset, FLASH_SECTOR_SIZE);
    memcpy(&sector_buf[flash_target_offset - aligned_sector_offset], buffer, sizeof(buffer));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(aligned_sector_offset - XIP_BASE, FLASH_SECTOR_SIZE);
    flash_range_program(aligned_sector_offset - XIP_BASE, sector_buf, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    log_message("WiFi credentials stored to flash.");
    return PICO_OK;
}

int erase_wifi_credentials(void) {
    uint8_t buffer[WIFI_CONFIG_SIZE] = {0};
    uint32_t flash_target_offset = (XIP_BASE + WIFI_CONFIG_OFFSET);
    uint32_t aligned_sector_offset = flash_target_offset & ~(FLASH_SECTOR_SIZE - 1);

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(aligned_sector_offset - XIP_BASE, FLASH_SECTOR_SIZE);
    flash_range_program(aligned_sector_offset - XIP_BASE, buffer, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    log_message("WiFi credentials erased from flash.");
    return PICO_OK;
}

int main() {
    boot_time_us = to_us_since_boot(get_absolute_time());
    log_message("Booting...");

    set_sys_clock_khz(250000, true); // Overclocks Pico to handle ROM emulation properly

    gpio_init_mask(ALL_GPIO_MASK);
    gpio_set_dir_in_masked(ALL_GPIO_MASK);

    multicore_launch_core1(core1_entry);

    if (wifi_init() != PICO_OK) {
        pico_state = PICO_STATE_WIFI_ERROR;
        error(3);
    }
    pico_set_led(true);

    stdio_usb_init();

    log_message("SVI-328 PicoROM version 1.0");

    if (fetch_wifi_credentials() != PICO_OK) {
        pico_state = PICO_STATE_WAITING_CREDENTIALS;
        log_message("Waiting for WiFi credentials...");

        while (true) {
            sleep_ms(100);
            if (SSID[0] != 0 && Password[0] != 0) {
                break;
            }
        }
        
        store_wifi_credentials((const char *)SSID, (const char *)Password);
    } 
    pico_state = PICO_STATE_CREDENTIALS_STORED;
    pico_set_led(false);
    pico_state = PICO_STATE_WIFI_CONNECTING;

    log_message("Connecting to WiFi...");
    int ret = wifi_connect(SSID, Password);
    if (ret != PICO_OK) {
        log_message("WiFi connect failed");
        pico_state = PICO_STATE_WIFI_ERROR;
        erase_wifi_credentials();
        error(2);
    }

    pico_state = PICO_STATE_WIFI_CONNECTED;
    log_message("WiFi connected to access point.");
    wait_for_ip();
    tcp_server_setup();

    pico_set_led(true);

    while (true) {
        sleep_ms(1000);
        if (!client_connected) {
            send_udp_broadcast();
        }
    }
}
