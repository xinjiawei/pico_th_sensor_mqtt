//
// Created by XINJIAWEI on 24-7-30.
//

#include "string"
#ifndef PICO_EXAMPLES_ECHO_UART_H
#define PICO_EXAMPLES_ECHO_UART_H

#endif //PICO_EXAMPLES_ECHO_UART_H

#define ECHO_BAUD_RATE 9600
#define UART_ECHO uart1
// logs echo
#define ECHO_UART_TX_PIN 8
#define ECHO_UART_RX_PIN 9

#define ECHO_LEVEL_FORCE 0
#define ECHO_LEVEL_AT_COMMAND 1
#define ECHO_LEVEL_DEBUG 2
#define ECHO_LEVEL_INFO 3
#define ECHO_LEVEL_WARNNING 4
#define ECHO_LEVEL_ERROR 5
using namespace std;

typedef const char * constcharp;

struct uartGetData{
    string data;
    int length;
};

void echo_uart_init();

bool change_echo_level(int level);

bool echo_uart(constcharp message, int level = ECHO_LEVEL_INFO);
bool get_uart(uartGetData * data);

std::string int2str(uint64_t index);
std::string float2str(float index);
