/**
 * SVI-328 PicoROM
 * 
 * Copyright (c) 2025 Markus Rautopuro
 * 
 * Works only with Raspberry Pico 2 W.
 */

#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"

#include "log.h"
#include "svi-328-cartridge.h"

#define TCP_PORT 4242
#define UDP_BROADCAST_PORT 4243

bool client_connected = false;
struct tcp_pcb *server_pcb;
size_t rom_offset = 0;
bool receiving_rom = false;

void pico_set_led(bool led_on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
}

void wait_for_ip() {
    struct netif *netif = &cyw43_state.netif[0];
    log_message("Waiting for IP address...");
    while (netif->ip_addr.addr == 0) {
        sleep_ms(100);
    }
    log_message("IP Address obtained: %s", ipaddr_ntoa(&netif->ip_addr));
}

int wifi_init() {
    return cyw43_arch_init();
}

int wifi_connect(char *ssid, char *password) {
    cyw43_arch_enable_sta_mode();
    return cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000);
}

void tcp_error_callback(void *arg, err_t err) {
    log_message("Client disconnected or connection error occurred, error: %d", err);
    client_connected = false;
}

err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        log_message("Client disconnected");
        tcp_close(tpcb);
        client_connected = false;
        receiving_rom = false;
        rom_offset = 0;
        return ERR_OK;
    }

    char *data = (char *)p->payload;

    if (receiving_rom) {
        size_t bytes_to_copy = UPLOADED_ROM_SIZE - rom_offset;
        if (p->len < bytes_to_copy) {
            memcpy(&uploaded_ROM[rom_offset], data, p->len);
            rom_offset += p->len;
        } else {
            memcpy(&uploaded_ROM[rom_offset], data, bytes_to_copy);
            rom_offset += bytes_to_copy;
            log_message("ROM upload complete (%zu bytes)", rom_offset);
            receiving_rom = false;
            pico_state = PICO_STATE_ROM_READY;
        }
    } else if (strncmp(data, "UPLOAD_ROM", 10) == 0) {
        log_message("Received UPLOAD_ROM command. Waiting for 32kB binary...");
        pico_state = PICO_STATE_RECEIVING_ROM;
        receiving_rom = true;
        rom_offset = 0;
    } else if (strncmp(data, "DUMP_LOG", 8) == 0) {
        log_message("Received DUMP_LOG command. Sending logs...");
        flush_log_buffer(server_pcb);
    } else {
        log_message("Received unknown command: %.*s", p->len, data);
    }

    tcp_recved(tpcb, p->len);
    pbuf_free(p);
    return ERR_OK;
}


err_t tcp_accept_callback(void *arg, struct tcp_pcb *new_pcb, err_t err) {
    client_connected = true;
    log_message("Client connected");
    pico_state = PICO_STATE_CLIENT_CONNECTED;

    server_pcb = new_pcb;

    tcp_recv(new_pcb, tcp_recv_callback);
    tcp_err(new_pcb, tcp_error_callback);
    return ERR_OK;
}

int tcp_server_setup() {
    struct netif *netif = &cyw43_state.netif[CYW43_ITF_STA];

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (!pcb) {
        log_message("Failed to create PCB");
        return -1;
    }

    err_t err = tcp_bind(pcb, &netif->ip_addr, TCP_PORT);
    if (err != ERR_OK) {
        log_message("TCP bind failed");
        return -1;
    }

    pcb = tcp_listen(pcb);
    tcp_accept(pcb, tcp_accept_callback);

    log_message("TCP server listening on %s port %d", ip4addr_ntoa(netif_ip4_addr(netif)), TCP_PORT);

    return ERR_OK;
}

void send_udp_broadcast() {
    struct netif *netif = &cyw43_state.netif[CYW43_ITF_STA];

    struct udp_pcb *udp = udp_new();
    if (!udp) {
        log_message("Failed to create UDP PCB");
        return;
    }

    ip_addr_t broadcast_addr = netif->ip_addr;
    ip4_addr_set_u32(&broadcast_addr, ip4_addr_get_u32(&broadcast_addr) | ~ip4_addr_get_u32(&netif->netmask));

    char msg[64];
    snprintf(msg, sizeof(msg), "SVI-328 PicoROM hello!");

    struct pbuf *pb = pbuf_alloc(PBUF_TRANSPORT, strlen(msg), PBUF_RAM);
    memcpy(pb->payload, msg, strlen(msg));

    udp_sendto(udp, pb, &broadcast_addr, UDP_BROADCAST_PORT);

    pbuf_free(pb);
    udp_remove(udp);
}
