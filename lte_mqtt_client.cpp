#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <cstdio>
#include <cstring>
#include <string>

#include "lte_mqtt_client.h"
#include "led_blink.h"
#include "echo_uart.h"

using namespace std;

/**
 * 串口命令发出
 * @param cmd
 * @param act
 * @param timeout
 * @return
 */
int lte_send_cmd(constcharp cmd, constcharp act, constcharp failed, uint64_t timeout_ms)
{

	int i = 0;
	char s[100] = "";
	char buffers[100] = {0};
	uint64_t t;

	echo_uart("\r\n------send-----\r\n", ECHO_LEVEL_DEBUG);
	echo_uart(cmd, ECHO_LEVEL_DEBUG);
	echo_uart("\r\n------receive-----\r\n", ECHO_LEVEL_DEBUG);

	uart_puts(UART_ID, cmd);
	uart_puts(UART_ID, "\r\n");

	t = time_us_64();
	//    echo_uart("------time test s-----\r\n");
	//    echo_uart(int2str(t).c_str());
	//    echo_uart("\r\n");
	//    echo_uart("------time test e-----\r\n");

	while (time_us_64() - t < timeout_ms * 1000)
	{
		while (uart_is_readable_within_us(UART_ID, 2000))
		{
			buffers[i++] = uart_getc(UART_ID);
		}

		if (i > 0)
		{
			buffers[i] = '\0';
			echo_uart(buffers, ECHO_LEVEL_DEBUG);
			echo_uart("\r\n", ECHO_LEVEL_DEBUG);
			if (strstr(buffers, act) != nullptr)
			{
				echo_uart("------(1-----\r\n", ECHO_LEVEL_DEBUG);
				return i;
			}
			else if (strstr(buffers, "ERROR"))
			{
				echo_uart("------(0------\r\n", ECHO_LEVEL_ERROR);
				return 0;
			}
			else
			{
				echo_uart(buffers, ECHO_LEVEL_WARNNING);
				echo_uart("------(-2------\r\n", ECHO_LEVEL_WARNNING);
				i = 0;
			}
		}
	}
	echo_uart("------(-1-----\r\n", ECHO_LEVEL_ERROR);
	// printf("false\r\n");
	return -1;
}

int init_lte()
{
	sleep_ms(7000);
	int send_result = 0;
	//********************************************************************************/
	echo_uart("*** lte set apn start ***\r\n", ECHO_LEVEL_INFO);
	// const char *sendtemp = "ATE0";
	send_result = lte_send_cmd("ATE0\r\n", "OK", "ERROR", 100);
	send_result = lte_send_cmd("AT+ICCID\r\n", "OK", "ERROR", 100);
	send_result = lte_send_cmd("AT+CSQ\r\n", "OK", "ERROR", 100);
	send_result = lte_send_cmd("AT+CEREG?\r\n", "OK", "ERROR", 100);
	
	
	send_result = lte_send_cmd("AT+QICSGP=1,1,\"cmnbiot\",\"\",\"\"\r\n", "OK", "ERROR", 100);
	// send_result = lte_send_cmd("AT+QICSGP=1,1,\"ctnb\",\"\",\"\"\r\n", "OK", "ERROR", 100);
	if (send_result >= 1)
	{
		echo_uart("lte set ok\r\n", ECHO_LEVEL_INFO);
	}
	else if (send_result == 0)
	{
		echo_uart("lte set error\r\n", ECHO_LEVEL_ERROR);
		return 0;
	}
	else
	{
		echo_uart("lte set unknown\r\n", ECHO_LEVEL_WARNNING);
		return 0;
	}
	echo_uart("*** lte set end ***\r\n", ECHO_LEVEL_INFO);
	//********************************************************************************/
	echo_uart("*** lte open data start ***\r\n", ECHO_LEVEL_INFO);
	send_result = lte_send_cmd("AT+NETOPEN\r\n", "SUCC", "FAIL", 7000);
	if (send_result >= 1)
	{
		echo_uart("lte set ok\r\n", ECHO_LEVEL_INFO);
	}
	else if (send_result == 0)
	{
		echo_uart("lte set error\r\n", ECHO_LEVEL_ERROR);
	}
	else
	{
		echo_uart("lte set unknown\r\n", ECHO_LEVEL_WARNNING);
	}
	echo_uart("*** lte open data end ***\r\n", ECHO_LEVEL_INFO);
	//********************************************************************************/
	echo_uart("*** lte set mqtt user start ***\r\n", ECHO_LEVEL_INFO);
	send_result = lte_send_cmd("AT+MCONFIG=\"lte001\",\"admin\",\"447797839\",0,0,0,\"\",\"\"\r\n", "OK", "ERROR", 300);
	if (send_result >= 1)
	{
		echo_uart("lte set ok\r\n", ECHO_LEVEL_INFO);
	}
	else if (send_result == 0)
	{
		echo_uart("lte set error\r\n", ECHO_LEVEL_ERROR);
		return 0;
	}
	else
	{
		echo_uart("lte set unknown\r\n", ECHO_LEVEL_WARNNING);
		return 0;
	}
	echo_uart("*** lte set mqtt user end ***\r\n", ECHO_LEVEL_INFO);
	//********************************************************************************/
	// vTaskDelay(1000 / portTICK_PERIOD_MS);
	echo_uart("*** lte set mqtt server start ***\r\n", ECHO_LEVEL_INFO);
	send_result = lte_send_cmd("AT+MIPSTART=\"hb01.mqtt.mb6.top\",18830,4\r\n", "OK", "ERROR", 200);
	if (send_result >= 1)
	{
		echo_uart("lte set ok\r\n", ECHO_LEVEL_INFO);
	}
	else if (send_result == 0)
	{
		echo_uart("lte set error\r\n", ECHO_LEVEL_ERROR);
		return 0;
	}
	else
	{
		echo_uart("lte set unknown\r\n", ECHO_LEVEL_WARNNING);
		return 0;
	}
	echo_uart("*** lte set mqtt server end ***\r\n", ECHO_LEVEL_INFO);
	//********************************************************************************/
	sleep_ms(1000);
	echo_uart("*** lte mqtt connect start ***\r\n", ECHO_LEVEL_INFO);
	send_result = lte_send_cmd("AT+MCONNECT=1,60\r\n", "SUC", "FAIL", 2000);
	if (send_result >= 1)
	{
		echo_uart("lte set ok\r\n", ECHO_LEVEL_INFO);
	}
	else if (send_result == 0)
	{
		echo_uart("lte set error\r\n", ECHO_LEVEL_ERROR);
		return 0;
	}
	else
	{
		echo_uart("lte set unknown\r\n", ECHO_LEVEL_WARNNING);
		return 0;
	}
	echo_uart("*** lte mqtt connect end ***\r\n", ECHO_LEVEL_INFO);
	return 1;
}

/**
 * 发送简单消息
 * @param topic
 * @param message
 * @return
 */
bool lte_mqtt_send_msg(constcharp topic, constcharp message)
{
	char message_str[255] = "";
	sprintf(message_str, "AT+MPUB=\"%s\",0,1,\"%s\"\r\n", topic, message);
	int send = lte_send_cmd(message_str, "SUCC", "ERROR");
	if (send <= 0)
	{
		return false;
	}
	return true;
}
