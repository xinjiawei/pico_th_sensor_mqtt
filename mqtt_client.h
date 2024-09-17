//
// Created by XINJIAWEI on 24-7-30.
//

#ifndef PICO_EXAMPLES_MQTT_CLIENT_H
#define PICO_EXAMPLES_MQTT_CLIENT_H

#endif //PICO_EXAMPLES_MQTT_CLIENT_H

#define UART_ID uart0

#define BAUD_RATE 115200

#define FLAG_VALUE 123

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

typedef const char * constcharp;

bool sendCMD(constcharp cmd,constcharp act,uint64_t timeout_ms=2000);
int esp8266_connect_wifi(constcharp ssid, constcharp passwd, int force=0);

bool esp8266_connect_init();
bool esp8266_mqtt_init(constcharp client_id,
                       constcharp username,
                       constcharp password,
                       constcharp host,
                       constcharp port,
                       constcharp will_topic,
                       constcharp will_msg);
bool esp8266_mqtt_send_msg(constcharp topic, constcharp message);
bool esp8266_mqtt_send_bin(constcharp topic, int length);
int esp8266_reset();
int esp8266_restore();

bool esp8266_smartconfig();