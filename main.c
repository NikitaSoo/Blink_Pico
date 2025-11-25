#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define BAUD_RATE 9600

#define BUTTON_PIN 9
#define BUFLEN 128

// Sends command and CRLF
void send_cmd(const char *cmd) {
    uart_puts(UART_ID, cmd);
    uart_puts(UART_ID, "\r\n");
}

// Reads one line from UART (simple student version)
int read_line(char *buf, int maxlen) {
    int i = 0;
    absolute_time_t timeout = make_timeout_time_ms(500);

    while (absolute_time_diff_us(get_absolute_time(), timeout) > 0) {
        if (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);

            if (c == '\n') {
                buf[i] = 0;
                return i;
            }

            if (i < maxlen - 1) {
                buf[i++] = c;
            }
        }
    }
    buf[i] = 0;
    return i;
}

// Remove colons and make lowercase
void clean_eui(char *str) {
    char tmp[64];
    int j = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] != ':' && str[i] != ',' && str[i] != '\r')
            tmp[j++] = tolower(str[i]);
    }
    tmp[j] = 0;
    strcpy(str, tmp);
}

int main() {
    stdio_init_all();
    sleep_ms(1500);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    char buf[BUFLEN];

    printf("LoRa UART ready. Press SW0.\n");

    while (1) {
        // Wait button press
        if (!gpio_get(BUTTON_PIN)) {
            while (!gpio_get(BUTTON_PIN)) {}
            printf("\n--- LoRa Query ---\n");

            //
            // 1) Test AT
            //
            send_cmd("AT");
            read_line(buf, BUFLEN);
            printf("AT reply: %s\n", buf);

            if (strstr(buf, "OK"))
                printf("Connected to LoRa module\n");
            else {
                printf("LoRa not responding\n");
                continue;
            }

            //
            // 2) Firmware version
            //
            send_cmd("AT+VER=?");
            read_line(buf, BUFLEN);
            printf("Firmware version: %s\n", buf);

            //
            // 3) DevEUI
            //
            send_cmd("AT+ID=DevEui");
            read_line(buf, BUFLEN);
            printf("Raw DevEUI: %s\n", buf);

            clean_eui(buf);
            printf("DevEUI: %s\n", buf);

            printf("--- Done ---\n");
        }

        // Print any unexpected messages from LoRa
        while (uart_is_readable(UART_ID)) {
            putchar(uart_getc(UART_ID));
        }
    }
}
