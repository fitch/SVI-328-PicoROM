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

#include "svi-328-cartridge.h"
#include "wifi.h"
#include "log.h"

#include "rom.h"

#define TOGGLE_SEQ_1 0x7DEA
#define TOGGLE_SEQ_2 0x7D0B
#define TOGGLE_SEQ_3 0x7EEF

#define MAX_WRITE_RECORD 256

typedef enum {
    WRITE_MODE_SSID = 0x7f00,
    WRITE_MODE_PASSWORD = 0x7f01,
    WRITE_MODE_TERMINATE = 0x7f02
} write_mode_t;

typedef enum {
    PICO_STATUS_READY = 0,
    PICO_STATUS_OK = 200,
    PICO_STATUS_WRITE_BUFFER_FULL = 253,
    PICO_STATUS_ERROR = 254,
    PICO_STATUS_UNKNOWN = 255
} pico_status_t;

volatile unsigned char SSID[SSID_MAX_LENGTH + 1];
volatile unsigned char Password[PASSWORD_MAX_LENGTH + 1];

volatile unsigned char write_record[MAX_WRITE_RECORD];
volatile int write_index = 0;
volatile write_mode_t write_mode = WRITE_MODE_TERMINATE;
volatile write_mode_t previous_write_mode = WRITE_MODE_TERMINATE;
volatile pico_status_t pico_status = PICO_STATUS_READY;
volatile pico_state_t pico_state = PICO_STATE_UNKNOWN;

unsigned char uploaded_ROM[UPLOADED_ROM_SIZE];

/**
 * 32 kB ROM emulation
 */

void __no_inline_not_in_flash_func(core1_entry)() {
    uint32_t addr;
    uint32_t pins;
    uint16_t command_sequence[4] = {0, 0, 0, 0};

    while (pico_state != PICO_STATE_ROM_READY) { // We will first server the PicoROM launcher (see rom folder in the repository)
        while (gpio_get(CCS1_PIN) & gpio_get(CCS2_PIN)) { // Wait until CCS1 or CCS2 is low
            tight_loop_contents();
        }

        pins = gpio_get_all() & ALL_GPIO_MASK;
        addr = pins & ADDRESS_PIN_MASK | ((pins & CCS1_GPIO_MASK) ? 1UL << 14 : 0); // Use CCS1 as A14

        command_sequence[0] = command_sequence[1];
        command_sequence[1] = command_sequence[2];
        command_sequence[2] = command_sequence[3];
        command_sequence[3] = addr;

        bool command_issued = false;

        if (command_sequence[0] == TOGGLE_SEQ_1 &&
            command_sequence[1] == TOGGLE_SEQ_2 &&
            command_sequence[2] == TOGGLE_SEQ_3) {  // ROM issued a write command

            command_issued = true;

            write_mode = command_sequence[3];

            switch (write_mode) {
            case WRITE_MODE_SSID:
                write_index = 0;
                memset((void*)write_record, 0, sizeof(write_record));
                break;
            case WRITE_MODE_PASSWORD:
                write_index = 0;
                memset((void*)write_record, 0, sizeof(write_record));
                break;
            case WRITE_MODE_TERMINATE:
                switch (previous_write_mode) {
                case WRITE_MODE_SSID:
                    if (write_index > 0) {
                        memcpy((void*)SSID, (void*)write_record, SSID_MAX_LENGTH);
                    }
                    break;
                case WRITE_MODE_PASSWORD:
                    if (write_index > 0) {
                        memcpy((void*)Password, (void*)write_record, PASSWORD_MAX_LENGTH);
                    }
                    break;
                default:
                    break;
                }
                pico_status = PICO_STATUS_OK;
                break;
            default:
                break;
            }
            previous_write_mode = write_mode;
        }

        unsigned char value;
        if (command_issued) { // ROM has issued a write command
            value = PICO_STATUS_OK;
        } else if (write_mode != WRITE_MODE_TERMINATE) { // ROM is writing data
            value = (unsigned char)(addr & 0xff);
            if (write_index < MAX_WRITE_RECORD) {
                write_record[write_index++] = value;
            } else {
                pico_status = PICO_STATUS_WRITE_BUFFER_FULL;
            }
        } else if (addr == 0x7fff) { // Special address to read the state of Pico
            value = pico_state;
        } else { // Normal ROM data access
            value = ROM[addr];
        }
        gpio_set_dir_out_masked(DATA_PIN_MASK);
        gpio_put_masked(DATA_PIN_MASK, ((uint32_t)value) << 14);

        while (!gpio_get(CCS1_PIN) | !gpio_get(CCS2_PIN)) { // Wait until CCS1 and CCS2 are high
            tight_loop_contents();
        }
        gpio_set_dir_in_masked(DATA_PIN_MASK);
    }

    while (true) { // Here, we are just serving the uploaded 32 kB ROM
        while (gpio_get(CCS1_PIN) & gpio_get(CCS2_PIN)) {
            tight_loop_contents();
        }

        pins = gpio_get_all() & ALL_GPIO_MASK;
        addr = pins & ADDRESS_PIN_MASK | ((pins & CCS1_GPIO_MASK) ? 1UL << 14 : 0);

        gpio_set_dir_out_masked(DATA_PIN_MASK);
        gpio_put_masked(DATA_PIN_MASK, ((uint32_t)uploaded_ROM[addr]) << 14);

        while (!gpio_get(CCS1_PIN) | !gpio_get(CCS2_PIN)) {
            tight_loop_contents();
        }
        gpio_set_dir_in_masked(DATA_PIN_MASK);
    }
}