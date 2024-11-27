//
// Created by XINJIAWEI on 24-7-30.
//

#include <cmath>
#include <cstdlib>
#include <cstdio>

#include "hardware/i2c.h"
#include "dht20.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/time.h"
#include "echo_uart.h"

int dht20_init() {
#if !defined(DHT20_USE_I2C) || !defined(DHT20_I2C_SDA_PIN) || !defined(DHT20_I2C_SCL_PIN)
    #warning i2c/bus_scan example requires a board with I2C pins
  puts("Default I2C pins were not defined");
#else
    // This example will use I2C0 on the default SDA and SCL pins (GP4, GP5 on a
    // Pico)
    i2c_init(DHT20_USE_I2C, 100 * 1000);
    gpio_set_function(DHT20_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DHT20_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(DHT20_I2C_SDA_PIN);
    gpio_pull_up(DHT20_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(DHT20_I2C_SDA_PIN, DHT20_I2C_SCL_PIN,GPIO_FUNC_I2C));
#endif
    // 上电后要等待不少于100ms
    sleep_ms(120);

    // 读取温湿度值之前，通过发送0x71获取一个字节的状态字
    uint8_t buf[1] = {DHT20_COMMAND_STATUS};
    i2c_write_blocking(DHT20_USE_I2C, DHT20_ADDRESS, buf, 1, true);
    i2c_read_blocking(DHT20_USE_I2C, DHT20_ADDRESS, buf, 1, false);

    // 如果状态字和0x18相与后不等于0x18
    if (buf[0] != DHT20_RETURN_STATUS_OK) {
        // 初始化0x1B、0x1C、0x1E寄存器
        uint8_t init_mem[2] = {0x00, 0x00};
        i2c_write_blocking(DHT20_USE_I2C, DHT20_REGISTER_1, init_mem, 3, false);
        i2c_write_blocking(DHT20_USE_I2C, DHT20_REGISTER_2, init_mem, 3, false);
        i2c_write_blocking(DHT20_USE_I2C, DHT20_REGISTER_3, init_mem, 3, false);

//        return EXIT_FAILURE;
    }

    // 等待10ms
    sleep_ms(20);
    return EXIT_SUCCESS;
}

int dht20_measure(dht20_measurement *measurement) {

    // 发送0xAC命令(触发测量)，此命令参数有两个字节，第一个字节为0x33，第二个字节为0x00。
    uint8_t buf[7] = {DHT20_TRIGGER_MEASUREMENT, DHT20_TRIGGER_MEASUREMENT_PARAM1,
                      DHT20_TRIGGER_MEASUREMENT_PARAM2};

    i2c_write_blocking(DHT20_USE_I2C, DHT20_ADDRESS, buf, 3, false);

    bool success = false;
    for (int i = 0; i < 5; i++) {
        sleep_ms(80);
        i2c_read_blocking(DHT20_USE_I2C, DHT20_ADDRESS, buf, 1, true);
        success = (buf[0] & DHT20_RETURN_STATUS_BUSY_MASK) == 0x0;
        if (success) {
            break;
        }
    }

    if (!success) {
        measurement->humidity = -1;
        measurement->temperature = -1;
        return EXIT_FAILURE;
    }

    i2c_read_blocking(DHT20_USE_I2C, DHT20_ADDRESS, buf, 7, false);

    int humidity_raw = (buf[1] << 12) | (buf[2] << 8) | (buf[3] >> 4);
    int temperature_raw = ((buf[3] << 16) | (buf[4] << 8) | buf[5]) & 0xfffff;

    measurement->humidity = (humidity_raw / powf(2, 20)) * 100;
    measurement->temperature = ((temperature_raw / powf(2, 20)) * 200) - 50;

    return EXIT_SUCCESS;
}

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

bool dht20_data_init(){
   return dht20_init();
}

dht20_measurement dht20_data_get() {
    dht20_measurement dht20_result;
    dht20_measure(&dht20_result);
    return dht20_result;
}