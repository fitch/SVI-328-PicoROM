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
#include "lwip/apps/http_client.h"

#define LOG_BUFFER_SIZE 8192

char log_buffer[LOG_BUFFER_SIZE];
size_t log_index = 0;

uint32_t boot_time_us = 0;

void log_message(const char *format, ...) {
    uint32_t elapsed_us = to_us_since_boot(get_absolute_time()) - boot_time_us;

    va_list args;
    char temp_buffer[256];
    char final_buffer[300];

    va_start(args, format);
    int len = vsnprintf(temp_buffer, sizeof(temp_buffer), format, args);
    va_end(args);

    if (len < 0) {
        return;
    }

    snprintf(final_buffer, sizeof(final_buffer), "[%012u] %s\n", elapsed_us, temp_buffer);

    len = strlen(final_buffer);
    if (log_index + len < LOG_BUFFER_SIZE - 1) {
        strcpy(&log_buffer[log_index], final_buffer);
        log_index += len;
    }
    printf("%s", final_buffer);
}

void flush_log_buffer(struct tcp_pcb *pcb) {
    char send_buffer[LOG_BUFFER_SIZE];
    size_t send_length;

    // FIXME: This might not be that thread-safe
    memcpy(send_buffer, log_buffer, log_index);
    send_length = log_index;
    log_index = 0;
    memset(log_buffer, 0, LOG_BUFFER_SIZE);

    size_t sent = 0;
    size_t remaining = send_length;

    while (remaining > 0) {
        uint16_t snd_buf = tcp_sndbuf(pcb);
        size_t chunk_size = remaining < snd_buf ? remaining : snd_buf;

        if (chunk_size == 0) {
            tcp_output(pcb);
            sleep_ms(10);
            continue;
        }

        err_t write_err = tcp_write(pcb, send_buffer + sent, chunk_size, TCP_WRITE_FLAG_COPY);
        if (write_err != ERR_OK) {
            log_message("tcp_write failed: %d", write_err);
            break;
        }

        sent += chunk_size;
        remaining -= chunk_size;
    }

    tcp_output(pcb);
}