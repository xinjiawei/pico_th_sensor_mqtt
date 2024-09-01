cmake_minimum_required(VERSION 3.12)
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico_th C CXX ASM)
pico_sdk_init()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_executable(senior
        main.h
        main.cpp
        bmp280_i2c.h
        bmp280_i2c.cpp
        mqtt_client.h
        mqtt_client.cpp
        led_blink.h
        led_blink.cpp
        echo_uart.h
        echo_uart.cpp
        dht20.h
        dht20.cpp
        crc_8_16.h
        crc_8_16.cpp
        lte_mqtt_client.h
        lte_mqtt_client.cpp)

# pull in common dependencies
target_link_libraries(senior pico_stdlib pico_multicore hardware_i2c)


# add url via pico_set_program_url
# example_auto_set_url(senior)

# enable usb output, disable uart output
pico_enable_stdio_usb(senior 0)
pico_enable_stdio_uart(senior 1)
	
# create map/bin/hex file etc.
pico_add_extra_outputs(senior)

