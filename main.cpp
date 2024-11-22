//
// Created by XINJIAWEI on 24-7-30.
//
#include <cstdio>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <regex>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "hardware/watchdog.h"

#include "main.h"
#include "echo_uart.h"
#include "bmp280_i2c.h"
#include "dht20.h"
#include "mqtt_client.h"
#include "lte_mqtt_client.h"
#include "led_blink.h"

using namespace std;

const char wifi_ssid[] = "PDCN";
const char wifi_password[] = "";

constcharp mqtt_server_host = "hb01.mqtt.mb6.top";
constcharp mqtt_server_tcp_port = "18830";
constcharp mqtt_server_ws_port = "18831";
constcharp mqtt_server_client_id = "node002";
constcharp mqtt_server_user = "";
constcharp mqtt_server_passwd = "";
constcharp mqtt_server_will_topic = "offline";
constcharp mqtt_server_will_message = "will disconnect";

/// \tag::multicore_fifo_irqs[]

static int core0_rx_val = 0, core1_rx_val = 0;

char echo_rx_buffers[100] = {0};

int sys_loop = 5;

int main_thread_step = 1;

void software_reset()
{
	sleep_ms(1500);
	esp8266_reset();
	echo_uart("software rebooting\r\n", ECHO_LEVEL_INFO);
    // Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
    // second arg is pause on debug which means the watchdog will pause when stepping through code
    watchdog_enable(100, 1);
    watchdog_update();

    // Wait in an infinite loop and don't update the watchdog so it reboots us
    while(1);
}

float get_loop_sleep()
{
	return sys_loop;
}

int set_loop_sleep(float f)
{
	sys_loop = f;
	return 1;
}

/**
 * 核心0执行触发器
 */
void core0_sio_irq() {
    // Just record the latest entry
    while (multicore_fifo_rvalid())
        core0_rx_val = multicore_fifo_pop_blocking();

    multicore_fifo_clear_irq();

    if(core0_rx_val == 0){
        // reboot
        software_reset();
    }else if(core0_rx_val == 1){

    }else{

    }

	echo_uart("------core0 trigger s-----\r\n", ECHO_LEVEL_DEBUG);
	echo_uart(int2str(core0_rx_val).c_str(), ECHO_LEVEL_DEBUG);
	echo_uart("\r\n", ECHO_LEVEL_DEBUG);
	echo_uart("------core0 trigger e-----\r\n", ECHO_LEVEL_DEBUG);
}

/**
 * 核心1执行触发器
 */
void core1_sio_irq() {
    // Just record the latest entry
    while (multicore_fifo_rvalid())
        core1_rx_val = multicore_fifo_pop_blocking();

    multicore_fifo_clear_irq();

	echo_uart("------core1 trigger s-----\r\n", ECHO_LEVEL_DEBUG);
	echo_uart(int2str(core1_rx_val).c_str(), ECHO_LEVEL_DEBUG);
	echo_uart("\r\n", ECHO_LEVEL_DEBUG);
	echo_uart("------core1 trigger e-----\r\n", ECHO_LEVEL_DEBUG);

}

/**
 * 核心1主函数
 */
void core1_entry() {
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_sio_irq);

    irq_set_enabled(SIO_IRQ_PROC1, true);

    // Send something to Core0, this should fire the interrupt.
    multicore_fifo_push_blocking(TRIGGER_COMMAND_FROM_CORE1);

    uartGetData init_get_data = {"",0};

    while (1){
        uartGetData *uart_get_data = &init_get_data;
        sleep_ms(1000);
        get_uart(uart_get_data);
        const char * all_cmd = (uart_get_data->data).c_str();
        int all_cmd_length = uart_get_data->length;
        if(all_cmd_length == 0){
            continue;
        }

		echo_uart("--------------------\r\nall_cmd: ", ECHO_LEVEL_DEBUG);
		echo_uart((uart_get_data->data).c_str(), ECHO_LEVEL_DEBUG);
		echo_uart("\r\ncmd_length: ", ECHO_LEVEL_DEBUG);
		echo_uart(int2str(all_cmd_length).c_str(), ECHO_LEVEL_DEBUG);
		echo_uart("\r\n", ECHO_LEVEL_DEBUG);
		echo_uart("--------------------\r\n", ECHO_LEVEL_DEBUG);


        // cut "e"
        char pure_cmd[all_cmd_length];
        strncpy(pure_cmd, all_cmd, all_cmd_length - 1);
        pure_cmd[all_cmd_length - 1] = '\0';

        // head
        char cmd_type[3] = {0};
        strncpy(cmd_type, pure_cmd, 2);
        cmd_type[2]  = '\0';

        // body
        char cmd_body[all_cmd_length - 2];
        strcpy(cmd_body, pure_cmd + 2);

        echo_uart("------\r\n", ECHO_LEVEL_INFO);
        echo_uart("\r\npure_cmd: ", ECHO_LEVEL_INFO);
        echo_uart(pure_cmd, ECHO_LEVEL_INFO);
        echo_uart("\r\ncmd_type: ", ECHO_LEVEL_INFO);
        echo_uart(cmd_type, ECHO_LEVEL_INFO);
        echo_uart("\r\ncmd_body: ", ECHO_LEVEL_INFO);
        echo_uart(cmd_body, ECHO_LEVEL_INFO);
        echo_uart("\r\n", ECHO_LEVEL_INFO);
        echo_uart("------\r\n", ECHO_LEVEL_INFO);

        uart_rx_blink();

        if(strcmp(cmd_type,"01") == 0){
            // reboot
            echo_uart("rebooting\r\n", ECHO_LEVEL_INFO);
            // setting_change_send_msg("setting", "reboot success");
            software_reset();
        }else if(strcmp(cmd_type,"02") == 0){
            #ifdef NET_ESP8266
			// connect wifi
			std::string str = cmd_body;

			std::regex ws_re("\\s+");
			std::vector<std::string> res(
				std::sregex_token_iterator(
					str.begin(), str.end(), ws_re, -1),
				std::sregex_token_iterator());

			const char *passwd = res[1].c_str();
			const char *ssid = res[0].c_str();

			echo_uart("\r\nssid: ", ECHO_LEVEL_INFO);
			echo_uart(ssid, ECHO_LEVEL_INFO);
			echo_uart("\r\npassword: ", ECHO_LEVEL_INFO);
			echo_uart(passwd, ECHO_LEVEL_INFO);
			echo_uart("\r\n", ECHO_LEVEL_INFO);

			int con = esp8266_connect_wifi(ssid, passwd, 1);
			if (!con)
			{
				blink(1, 1, 1, 2, 2);
			}
			//            esp8266_reset();
			echo_uart("esp8266 wifi init ok\r\n", ECHO_LEVEL_INFO);
			// reboot
			software_reset();
            #elif defined(NET_LTE)
            #else
            #warning net not int
            #endif // NET_ESP8266

        }else if(strcmp(cmd_type,"03") == 0){
            // change log level
            echo_uart("\r\nwill change level to: ", ECHO_LEVEL_FORCE);
			echo_uart(cmd_body, ECHO_LEVEL_FORCE);
			echo_uart("\r\n", ECHO_LEVEL_FORCE);

            change_echo_level(ECHO_LEVEL_AT_COMMAND);
            sleep_ms(1000);

            echo_uart("+++\r\n", ECHO_LEVEL_AT_COMMAND);
            sleep_ms(1000);
            char temp[20] = "";
            sprintf(temp, "AT+LEVEL%s\r\n", cmd_body);
            echo_uart(temp, ECHO_LEVEL_AT_COMMAND);
            sleep_ms(1000);

            echo_uart("+++\r\n", ECHO_LEVEL_AT_COMMAND);
            sleep_ms(1000);
            software_reset();
        }else if(strcmp(cmd_type,"04") == 0){
            #ifdef NET_ESP8266
            // restore wifi
            //esp8266_restore();
			// enter smart config
			main_thread_step = 0;
			sleep_ms(1500);
			esp8266_smartconfig();
			software_reset();
#elif defined(NET_LTE)
            #else
            #warning net not int
            #endif // NET_ESP8266
		}
	    else if (strcmp(cmd_type, "05") == 0)
		{
			// change flush time
			if (strcmp(cmd_body, "1") == 0)
			{
				set_loop_sleep(1);
			}
			else if (strcmp(cmd_body, "2") == 0)
			{
				set_loop_sleep(2);
			}
			else if (strcmp(cmd_body, "3") == 0)
			{
				set_loop_sleep(3);
			}
			else if (strcmp(cmd_body, "4") == 0)
			{
				set_loop_sleep(4);
			}
			else if (strcmp(cmd_body, "5") == 0)
			{
				set_loop_sleep(5);
			}
			else if (strcmp(cmd_body, "6") == 0)
			{
				set_loop_sleep(0.1);
			}
			else if (strcmp(cmd_body, "7") == 0)
			{
				set_loop_sleep(0.3);
			}
			else if (strcmp(cmd_body, "8") == 0)
			{
				set_loop_sleep(0.5);
			}
			else
			{
				echo_uart("change loop param error: ", ECHO_LEVEL_INFO);
				echo_uart(cmd_body, ECHO_LEVEL_INFO);
				echo_uart("\r\n", ECHO_LEVEL_INFO);
				init_get_data = {"", 0};
				continue;
			}
			echo_uart("change loop time to ", ECHO_LEVEL_INFO);
			echo_uart(cmd_body, ECHO_LEVEL_INFO);
			echo_uart("\r\n", ECHO_LEVEL_INFO);
		}else{
            echo_uart("unknown cmd", ECHO_LEVEL_INFO);
            echo_uart("\r\n01e #reboot\r\n02PDCN 15033678058e #connect wifi\r\n030e #change lora LEVEL\r\n04e #wifi smartconfig\r\n051e #loop wait", ECHO_LEVEL_INFO);
            echo_uart("\r\n", ECHO_LEVEL_INFO);
            echo_uart("\r\n", ECHO_LEVEL_INFO);
        }
        init_get_data = {"",0};
    }
}

//// I2C reserves some addresses for special purposes. We exclude these from the scan.
//// These are any addresses of the form 000 0xxx or 111 1xxx
//bool reserved_addr(uint8_t addr) {
//    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
//}
/**
 * i2c扫描函数
 */
void i2c_scan(){
    // Enable UART so we can print status output
    //stdio_init_all();
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/bus_scan example requires a board with I2C pins
    puts("Default I2C pins were not defined");
#else
    // This example will use I2C0 on the default SDA and SCL pins (GP4, GP5 on a Pico)
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

	echo_uart("\r\nI2C Bus Scan\r\n", ECHO_LEVEL_DEBUG);
	echo_uart("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n", ECHO_LEVEL_DEBUG);

    char aaa[50] = "";
    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            sprintf(aaa, "%02x ", addr);
			echo_uart(aaa, ECHO_LEVEL_DEBUG);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

		echo_uart(ret < 0 ? "." : "@", ECHO_LEVEL_DEBUG);
		echo_uart(addr % 16 == 15 ? "\r\n" : "  ", ECHO_LEVEL_DEBUG);
    }
	echo_uart("Done.\r\n", ECHO_LEVEL_DEBUG);
    blink(1,1,2,2,2);
#endif
}

/**
 * 程序入口+核心0主函数
 * @return
 */
int main() {
    stdio_init_all();
    sleep_ms(5000);

    blink(2);

    echo_uart_init();
    if (watchdog_caused_reboot()) {
        echo_uart("\r\nwatchdog boot ok\r\n", ECHO_LEVEL_INFO);
    } else {
        echo_uart("\r\nboot ok\r\n", ECHO_LEVEL_INFO);
    }

    echo_uart("echo_uart_init ok\r\n", ECHO_LEVEL_INFO);

#ifdef NET_ESP8266
	// todo esp8266
    bool init = esp8266_connect_init();
    if(!init){
        blink(1,1,1,1,2);
    }
    echo_uart("esp8266 init ok\r\n", ECHO_LEVEL_INFO);

    // sendCMD("AT+RST","OK");
    // sleep_ms(2000);

    int con = esp8266_connect_wifi(wifi_ssid, wifi_password);
    if(!con){
		echo_uart("esp8266 wifi connect error\r\n", ECHO_LEVEL_ERROR);
        blink(1,1,1,2,2,true);

		esp8266_smartconfig();
		software_reset();
    }
    echo_uart("esp8266 wifi connect ok\r\n", ECHO_LEVEL_INFO);

    // 看门狗触发
//    if (!watchdog_caused_reboot()) {
    if (1){
        bool mqtt_init = esp8266_mqtt_init(mqtt_server_client_id,mqtt_server_user, mqtt_server_passwd,mqtt_server_host, mqtt_server_tcp_port, mqtt_server_will_topic, mqtt_server_will_message);
        if(!mqtt_init){
			echo_uart("esp8266 mqtt init error\r\n", ECHO_LEVEL_ERROR);
            blink(1,2,1,2,2);
        }
        echo_uart("esp8266 mqtt init ok\r\n", ECHO_LEVEL_INFO);
    }

    bool send = esp8266_mqtt_send_msg("online", mqtt_server_client_id);
    if(!send){
		echo_uart("esp8266 mqtt send test error\r\n", ECHO_LEVEL_ERROR);
        //blink(2,1,1,1,1);
    }else{
        echo_uart("esp8266 mqtt send test ok\r\n", ECHO_LEVEL_INFO);
    }

/*
    bool send_bin = esp8266_mqtt_send_bin("temperature", ECHO_LEVEL_INFO);
    if(!send_bin){
        blink(2,2,1,1,1);
    }
     */
#elif defined(NET_LTE)
	echo_uart("lte init start\r\n", ECHO_LEVEL_INFO);
	int init_lte_result = init_lte();
	if (!init_lte_result)
	{
		echo_uart("lte init error\r\n", ECHO_LEVEL_ERROR);
		blink(2, 2, 2, 1, 2);
	}
	echo_uart("lte init success\r\n", ECHO_LEVEL_INFO);
#else
#warning net not init
#endif // NET_ESP8266

    // todo 气压传感器
    echo_uart("bmp280 init start\r\n", ECHO_LEVEL_INFO);
    bool init_bmp280 = init_pressureS();
    if(!init_bmp280){
		echo_uart("bmp280 init error\r\n", ECHO_LEVEL_ERROR);
        blink(2,2,1,1,1);
    }
    echo_uart("bmp280 init success\r\n", ECHO_LEVEL_INFO);

    // todo dht20

    echo_uart("dht20 init start\r\n", ECHO_LEVEL_INFO);
    bool dht20_init = dht20_data_init();
    if(dht20_init){
		echo_uart("dht20 init error\r\n", ECHO_LEVEL_ERROR);
        exit(0);
    }
    echo_uart("dht20 init success\r\n", ECHO_LEVEL_INFO);

// todo multicore
    echo_uart("mult_core start loading\r\n", ECHO_LEVEL_INFO);

    // We MUST start the other core before we enabled FIFO interrupts.
    // This is because the launch uses the FIFO's, enabling interrupts before
    // they are used for the launch will result in unexpected behaviour.
    multicore_launch_core1(core1_entry);

    irq_set_exclusive_handler(SIO_IRQ_PROC0, core0_sio_irq);
    irq_set_enabled(SIO_IRQ_PROC0, true);

    // Wait for a bit for things to happen
    sleep_ms(15);

    // Send something back to the other core
    multicore_fifo_push_blocking(TRIGGER_COMMAND_FROM_CORE0);

    // Wait for a bit for things to happen
    sleep_ms(1000);

    int count = 1;

	float timee = get_loop_sleep();
	while (true){
		sleep_ms(1000 * timee);
		if (!main_thread_step)
		{
			continue;
		}
		dht20_measurement tempdht20_data = dht20_data_get();
        float temperature = tempdht20_data.temperature;
        float humidity = tempdht20_data.humidity;
        bmp280Reading bmp280_data =  get_pressureS();

        float p_temperature = bmp280_data.temp_c;
        float p_pressure = bmp280_data.pressure_kpa;

		/*
		echo_uart(int2str(count).c_str(), ECHO_LEVEL_DEBUG);
		echo_uart("\r\n", ECHO_LEVEL_DEBUG);
		echo_uart(float2str(temperature).c_str(), ECHO_LEVEL_DEBUG);
		echo_uart("\r\n", ECHO_LEVEL_DEBUG);
		echo_uart(float2str(humidity).c_str(), ECHO_LEVEL_DEBUG);
		echo_uart("\r\n", ECHO_LEVEL_DEBUG);
		 **/


        char mqtt_msg[100] = "";
        #ifdef NET_ESP8266
		sprintf(mqtt_msg, R"({\"n\":\"%02X\"\,\"t\":\"%.1f\"\,\"h\":\"%.1f\"\,\"tp\":\"%.1f\"\,\"p\":\"%.2f\"\})", count, temperature, humidity, p_temperature, p_pressure);
		bool send = esp8266_mqtt_send_msg("dht20", mqtt_msg);
        #elif defined(NET_LTE)
		sprintf(mqtt_msg, "{\"n\":\"%02X\",\"t\":\"%.1f\",\"h\":\"%.1f\",\"tp\":\"%.1f\",\"p\":\"%.2f\"}", count, temperature, humidity, p_temperature, p_pressure);
		bool send = lte_mqtt_send_msg("dht20", mqtt_msg);
        #else
        #warning net not int
        #endif // NET_ESP8266

        if(!send){
			echo_uart("mqtt msg send error\r\n", ECHO_LEVEL_ERROR);
        }
		echo_uart(mqtt_msg, ECHO_LEVEL_DEBUG);
		echo_uart("\r\n", ECHO_LEVEL_DEBUG);
        count++;
        blink();
		if (get_loop_sleep() != timee)
		{
			timee = get_loop_sleep();
		}
	}

    while (1){
        tight_loop_contents();
    }

}
