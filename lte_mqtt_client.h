#pragma once
#define UART_ID uart0

#define BAUD_RATE 115200

#define FLAG_VALUE 123

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

typedef const char *constcharp;
int init_lte();
int lte_send_cmd(constcharp cmd, constcharp act = "OK", constcharp failed = "ERROR", uint64_t timeout_ms = 100);

bool lte_mqtt_send_msg(constcharp topic, constcharp message);