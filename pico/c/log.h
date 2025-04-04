/**
 * SVI-328 PicoROM
 * 
 * Copyright (c) 2025 Markus Rautopuro
 * 
 * Works only with Raspberry Pico 2 W.
 */

void log_message(const char *format, ...);
extern uint32_t boot_time_us;

void flush_log_buffer(struct tcp_pcb *pcb);