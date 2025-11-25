#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart1
#define UART_TX_PIN 4   // Pico -> LoRa
#define UART_RX_PIN 5   // LoRa -> Pico
#define BAUD_RATE 9600

#define BUTTON_PIN 9   // SW0

int main() {
    stdio_init_all();
    sleep_ms(2000);

    printf("Boot\n");
    printf("Waiting SW0...\n");

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    while (true) {
        if (!gpio_get(BUTTON_PIN)) {
            while (!gpio_get(BUTTON_PIN)) {}
            printf("Sending AT...\n");
            uart_puts(UART_ID, "AT\r\n");
        }

        while (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);
            putchar(c);
        }
    }
}
