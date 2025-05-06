#include <cstdio>
#include <cstring>
#include <string>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

#include "mqtt_client.h"

#include "led_blink.h"

#include "echo_uart.h"

using namespace std;

char s[100]="";
char buffers[100] = {0};

/**
 * 串口命令发出
 * @param cmd
 * @param act
 * @param timeout
 * @return
 */
bool sendCMD(constcharp cmd,constcharp act,uint64_t timeout_ms) {
	
	int i=0;
	uint64_t t;

    echo_uart("\r\n------send-----\r\n", ECHO_LEVEL_DEBUG);
	echo_uart(cmd, ECHO_LEVEL_DEBUG);
	echo_uart("\r\n------receive-----", ECHO_LEVEL_DEBUG);

	uart_puts(UART_ID, cmd);
	uart_puts(UART_ID, "\r\n");
	
	t = time_us_64();
	//    echo_uart("------time test s-----\r\n", ECHO_LEVEL_DEBUG);
	//    echo_uart(int2str(t).c_str(), ECHO_LEVEL_DEBUG);
	//    echo_uart("\r\n", ECHO_LEVEL_DEBUG);
	//    echo_uart("------time test e-----\r\n", ECHO_LEVEL_DEBUG);


    while(time_us_64() - t < timeout_ms * 1000){
        while(uart_is_readable_within_us(UART_ID,2000)){
            buffers[i++] = uart_getc(UART_ID);
        }

        if(i>0){
            buffers[i]='\0';
			echo_uart(buffers, ECHO_LEVEL_DEBUG);
			echo_uart("\r\n", ECHO_LEVEL_DEBUG);
            if(strstr(buffers,act) != nullptr){
                echo_uart("------(1-----\r\n", ECHO_LEVEL_DEBUG);
                return true;
            }else if(strstr(buffers,"ERROR")){
                echo_uart("------(-1------\r\n", ECHO_LEVEL_DEBUG);
                return false;
            }else{
                echo_uart(buffers, ECHO_LEVEL_DEBUG);
                echo_uart("------(-2------\r\n", ECHO_LEVEL_DEBUG);
                i=0;
            }
        }
    }
    echo_uart("------(0-----\r\n", ECHO_LEVEL_DEBUG);
	//printf("false\r\n");
	return false;
}

/**
 * 连接wifi
 * @param ssid
 * @param passwd
 * @return
 */
int esp8266_connect_wifi(constcharp ssid, constcharp passwd, const int force){

    if(!force){
        bool last_con = sendCMD("AT+CWJAP","OK",20000);
        if(last_con){
            return true;
        }
    }
    sendCMD("AT+CWMODE=1","OK");

    sprintf(s,R"(AT+CWJAP="%s","%s")",ssid,passwd);
    bool re = sendCMD(s,"OK",15000);
    if(!re){
        return false;
    }
    return true;
}

int esp8266_reset(){
    return sendCMD("AT+RST","OK");
}

int esp8266_restore(){
    return sendCMD("AT+RESTORE","OK");
}

bool esp8266_smartconfig()
{
	echo_uart("enter smart config start 60s\r\n", ECHO_LEVEL_INFO);
	if (sendCMD("AT+CWSTARTSMART=1", "smartconfig connected wifi", 60000))
	{
		echo_uart("enter smart config ok\r\n", ECHO_LEVEL_INFO);
		return true;
	}
	else
	{
		echo_uart("enter smart config error\r\n", ECHO_LEVEL_INFO);
		return false;
	}
}

/*
 * N102=3,restart,1000
N2=A,AT+RST

N103=2,连接模式,1000
N3=A,AT+CWMODE=1

N104=0,连接,1000
N4=A,AT+CWJAP="PDCN""15033678058"

N105=0,上次连接,1000
N5=A,AT+CWJAP

N106=0,扫描,1000
N6=A,AT+CWLAP

N107=0, 断开链接,1000
N7=A,AT+CWQAP

N108=0,,1000
N8=A,

N109=0,mqtt user info,1000
N9=A,AT+MQTTUSERCFG=01"node-001""admin""447797839"00""

N110=0,topic info,1000
N10=A,AT+MQTTCONNCFG=0100"temperature""-1"01

N111=0,host info,1000
N11=A,AT+MQTTCONN=0"hb01.mqtt.mb6.top"188301

N112=0,check link status,1000
N12=A,AT+MQTTCONN?

N113=0,send message,1000
N13=A,AT+MQTTPUB=0"temperature""25.6"01

N114=0,send bin message,1000
N14=A,AT+MQTTPUBRAW=0"temperature"1001

N115=0,sub a topic,1000
N15=A,AT+MQTTSUB=0"main"0

N116=0,list subs,1000
N16=A,AT+MQTTSUB?

N117=0,cancel sub,1000
N17=A,AT+MQTTUNSUB=0"main"

N118=0,disconnect mqtt,1000
N18=A,AT+MQTTCLEAN=0
 */
/**
 * 初始化esp8266串口连接
 * @return
 */
bool esp8266_connect_init(){
    // Set up our UART with the required speed.
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_puts(UART_ID, "+++\r\n");
    uart_puts(UART_ID, "ATE0\r\n");
    sleep_ms(1000);
    while(uart_is_readable(UART_ID)) uart_getc(UART_ID);

    bool res = sendCMD("AT","OK");
	if (!res)
	{
		return false;
	}
	res = sendCMD("AT+CWMODE=1", "OK");
    if(!res){
        return false;
    }
    return true;
}

/**
 * 初始化mqtt
 * @param client_id
 * @param username
 * @param password
 * @param host
 * @param port
 * @param will_topic
 * @param will_msg
 * @return
 */
bool esp8266_mqtt_init(constcharp client_id,
                       constcharp username,
                       constcharp password,
                       constcharp host,
                       constcharp port,
                       constcharp will_topic,
                       constcharp will_msg){
    //
    char user_info_str[100] = "";
    char topic_info_str[100] = "";
    char host_info_str[100] = "";
    sprintf(user_info_str,R"(AT+MQTTUSERCFG=0,1,"%s","%s","%s",0,0,"")", client_id, username, password);
    bool init_user_info = sendCMD(user_info_str,"OK");
    if(!init_user_info){
        return false;
    }

    char mqtt_link_time_out[3] = "10";
    sprintf(topic_info_str,R"(AT+MQTTCONNCFG=0,%s,0,"%s","%s",0,1)", mqtt_link_time_out, will_topic, will_msg);
    bool init_topic_info = sendCMD(topic_info_str,"OK");
    if(!init_topic_info){
        return false;
    }
    sprintf(host_info_str,"AT+MQTTCONN=0,\"%s\",%s,1", host, port);
    bool init_host_info = sendCMD(host_info_str,"OK");
    if(!init_host_info){
        return false;
    }
    return true;
}

/**
 * 发送简单消息
 * @param topic
 * @param message
 * @return
 */
bool esp8266_mqtt_send_msg(constcharp topic, constcharp message){
    char message_str[255] = "";
    sprintf(message_str,R"(AT+MQTTPUB=0,"%s","%s",0,1)", topic, message);
    bool send = sendCMD(message_str,"OK");
    if(!send){
        return false;
    }
    return true;
}

/**
 * 发送二进制消息
 * @param topic
 * @param length
 * @return
 */
bool esp8266_mqtt_send_bin(constcharp topic, int length){
    char message_str[255] = "";
    sprintf(message_str,"AT+MQTTPUBRAW=0,\"%s\",%i,0,1", topic, length);
    bool send = sendCMD(message_str,"OK");
    if(!send){
        return false;
    }

    uart_puts(UART_ID, "3000");
    return true;
}