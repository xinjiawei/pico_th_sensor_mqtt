//
// Created by XINJIAWEI on 24-7-30.
//

#ifndef PICO_EXAMPLES_DNT20_H
#define PICO_EXAMPLES_DNT20_H
#endif //PICO_EXAMPLES_DNT20_H

#ifndef DHT20_H
#define DHT20_H

#define DHT20_USE_I2C i2c0
#define PICO_DEFAULT_I2C 0

#define DHT20_I2C_SDA_PIN 20
#define DHT20_I2C_SCL_PIN 21

#define DHT20_ADDRESS 0x38
#define DHT20_COMMAND_STATUS 0x71
#define DHT20_RETURN_STATUS_OK 0x18
#define DHT20_TRIGGER_MEASUREMENT 0xAC
#define DHT20_TRIGGER_MEASUREMENT_PARAM1 0x33
#define DHT20_TRIGGER_MEASUREMENT_PARAM2 0x00
#define DHT20_RETURN_STATUS_BUSY_MASK 0x80

#define DHT20_REGISTER_1 0x1B
#define DHT20_REGISTER_2 0x1C
#define DHT20_REGISTER_3 0x1E

typedef struct {
    float humidity;
    float temperature;
} dht20_measurement;

bool reserved_addr(uint8_t addr);
int dht20_init();
int dht20_measure(dht20_measurement *measurement);
bool dht20_data_init();
dht20_measurement dht20_data_get();

#endif // DHT20_H
