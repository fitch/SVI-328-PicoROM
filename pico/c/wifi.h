/**
 * SVI-328 PicoROM
 * 
 * Copyright (c) 2025 Markus Rautopuro
 * 
 * Works only with Raspberry Pico 2 W.
 */


void wait_for_ip(); 
void pico_set_led(bool led_on);
int wifi_connect();
int wifi_init();
int tcp_server_setup();

extern bool client_connected;
extern struct tcp_pcb *server_pcb;

void send_udp_broadcast();