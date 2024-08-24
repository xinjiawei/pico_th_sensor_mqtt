//
// Created by XINJIAWEI on 24-7-30.
//
#include "pico/stdlib.h"
/**
 * led闪烁
 * @param a
 * @param b
 * @param c
 * @param d
 * @param e
 * @param once
 */
void blink(int a,int b, int c, int d, int e, bool once){
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    if(a<= 0){
        return;
    }
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    int open_time = 500;
    int close_time = 300;
    int loop_wait_time = 2000;
    bool loop = true;
    while (loop) {
        // 1
        gpio_put(LED_PIN, true);
        sleep_ms(open_time * a);
        gpio_put(LED_PIN, false);
        // 2
        if(b * c * d * e != 0){
            sleep_ms(close_time);

            gpio_put(LED_PIN, true);
            sleep_ms(open_time * b);
            gpio_put(LED_PIN, false);
            sleep_ms(close_time);
            // 3
            gpio_put(LED_PIN, true);
            sleep_ms(open_time * c);
            gpio_put(LED_PIN, false);
            sleep_ms(close_time);
            // 4
            gpio_put(LED_PIN, true);
            sleep_ms(open_time * d);
            gpio_put(LED_PIN, false);
            sleep_ms(close_time);
            // 5
            gpio_put(LED_PIN, true);
            sleep_ms(open_time * e);
            gpio_put(LED_PIN, false);
            sleep_ms(loop_wait_time);
        }else{
            // 短暂闪烁不循环
            loop = false;
        }

        if(once){
            loop = false;
        }
    }
#endif
}

void uart_rx_blink(){
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    int open_time = 150;
    int close_time = 150;

    // 1
    int i = 1;
    do{
        gpio_put(LED_PIN, true);
        sleep_ms(open_time);
        gpio_put(LED_PIN, false);
        sleep_ms(close_time);
        i++;
    } while (i > 3);

    // 5
    gpio_put(LED_PIN, true);
    sleep_ms(open_time);
    gpio_put(LED_PIN, false);
#endif
}
