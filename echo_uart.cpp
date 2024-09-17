//
// Created by XINJIAWEI on 24-7-30.
//

#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pico/binary_info.h"
#include "echo_uart.h"
using namespace std;

int ECHO_LEVEL = ECHO_LEVEL_INFO;

/*
 * 初始化输出打印
 */
void echo_uart_init() {
    // Set up our UART with the required speed.
    uart_init(UART_ECHO, ECHO_BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(ECHO_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(ECHO_UART_RX_PIN, GPIO_FUNC_UART);
}

bool change_echo_level(int level){
    ECHO_LEVEL = level;
    return true;
}

/**
 * 打印错误信息
 * @param message
 * @return
 */
bool echo_uart(constcharp message, int level){
	if (level <= 1) // ECHO_LEVEL_FORCE and ECHO_LEVEL_AT_COMMAND
	{
		uart_puts(UART_ECHO, message);
		return true;
	}
	else if (level >= ECHO_LEVEL)
	{
		uart_puts(UART_ECHO, message);
		return true;
	}
	return false;
}

/**
 * 获取串口数据
 * @param timeout_ms
 * @return
 */
bool get_uart(uartGetData * data) {
    int i=0;
    char rx_buffers[100] = {0};

	echo_uart("read uart message\r\n", ECHO_LEVEL_DEBUG);
    while(uart_is_readable_within_us(UART_ECHO,20)){
        rx_buffers[i++] = uart_getc(UART_ECHO);

        if(i >= 3){
            if(strcmp(&rx_buffers[i-1], "e") == 0){
				echo_uart("ok", ECHO_LEVEL_DEBUG);
				echo_uart("\r\n", ECHO_LEVEL_DEBUG);
                data->data = rx_buffers;
                data->length = i;
                return true;
            }
        }
    }
    return false;
}

/**
 * int to const char pointer
 * @param filenameIndex
 * @return
 */
std::string int2str(uint64_t filenameIndex){
    stringstream temp_str;
    temp_str<<(filenameIndex);
    return temp_str.str();
}

std::string float2str(float index){
    std::stringstream temp_str;
    temp_str << std::fixed << std::setprecision(3) << index;
    return temp_str.str();
}
