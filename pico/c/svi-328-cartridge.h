/**
 * SVI-328 PicoROM
 * 
 * Copyright (c) 2025 Markus Rautopuro
 * 
 * Works only with Raspberry Pico 2 W.
 */

/*
    A0..A13 -> GP0..13, 2^14 = 16484
    D0..D7 -> GP14..21
    CCS1 -> GP22
    CCS2 -> GP26
    CCS3 -> GP27
    CCS4 -> GP28
*/

#define CCS1_PIN 22
#define CCS2_PIN 26
#define CCS3_PIN 27
#define CCS4_PIN 28

#define CCS1_GPIO_MASK (1UL << CCS1_PIN)
#define CCS2_GPIO_MASK (1UL << CCS2_PIN)
#define CCS3_GPIO_MASK (1UL << CCS3_PIN)
#define CCS4_GPIO_MASK (1UL << CCS4_PIN)

#define CCS_GPIO_MASK (CCS1_GPIO_MASK | CCS2_GPIO_MASK | CCS3_GPIO_MASK | CCS4_GPIO_MASK)
#define ADDRESS_PIN_MASK (1UL << 14) - 1        // first 14 bits
#define DATA_PIN_MASK ((1UL << 8) - 1) << 14    // second 8 bits
#define ALL_GPIO_MASK (ADDRESS_PIN_MASK | DATA_PIN_MASK | CCS_GPIO_MASK)

#define SSID_MAX_LENGTH 32
#define PASSWORD_MAX_LENGTH 32

#define UPLOADED_ROM_SIZE 32768

typedef enum {
    PICO_STATE_WAITING_CREDENTIALS = 100,
    PICO_STATE_CREDENTIALS_STORED = 101,
    PICO_STATE_WIFI_CONNECTING = 103,
    PICO_STATE_WIFI_CONNECTED = 104,
    PICO_STATE_WIFI_ERROR = 105,
    PICO_STATE_CLIENT_CONNECTED = 106,
    PICO_STATE_RECEIVING_ROM = 107,
    PICO_STATE_ROM_READY = 108,
    PICO_STATE_ERROR = 254,
    PICO_STATE_UNKNOWN = 255
} pico_state_t;

void __no_inline_not_in_flash_func(core1_entry)();

extern volatile pico_state_t pico_state;
extern volatile unsigned char SSID[];
extern volatile unsigned char Password[];
extern unsigned char uploaded_ROM[];