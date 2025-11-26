#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/unistd.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"

//Uart muuttujat:
#define LORA uart1
#define TX_PIN 4
#define RX_PIN 5
#define BAUDRATE 9600

//SW0 nappi:
#define BUTTON 9

//Maksimi rivin pituus:
#define ROW_LEN 128


//Funktiot joita käytetään:
bool read_line(char *buffer, int max, int timeout_ms);
void send_cmd(const char *cmd);
void format_eui(char *output, const char *input);

int main() {
    stdio_init_all();
    sleep_ms(2000);


    //UART1 LoRa-moduulia varten:
    uart_init(LORA, BAUDRATE);
    gpio_set_function(TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RX_PIN, GPIO_FUNC_UART);


    //Nappi:
    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);

    char line[ROW_LEN];
    char cleaned[32];

    while (1) {

        //Odotellaan kunnes painetaan nappia:
        printf("Paina nappia aloittaaksesi!\n");
        while (gpio_get(BUTTON)) {
            sleep_ms(20);
        }

        while (!gpio_get(BUTTON)) {
            sleep_ms(20);
        }

        //Lähetetään AT komento (maximi 5 yritystä):
        printf("Yhdistetaan LORA...\n");
        bool SUCCESS = false;

        for (int i = 0; i < 5 && !SUCCESS; i++) {

            send_cmd("AT");


            for (int j = 0; j < 3; j++) {
                if (read_line(line, ROW_LEN, 500)) {
                    printf("Vastaanotetaan: %s\n", line);

                    //Jos yhteys löytyy niin tulostetaan "OK" ja yhteys on kunnossa:
                    if (strstr(line, "OK")) {
                        SUCCESS = true;
                    }
                }
            }

            if (!SUCCESS) {
                printf("Ei vastausta, kokeillaan uudelleen...\n");
            }
        }

        //Jos jostain kumman syystä ei saatu yhtäkään "OK" vastausta niin yhteys ei toimi:
        if (!SUCCESS) {
            printf("LORA ei toimi :).\n\n");

            //Continue todella tärkeä, koska tehtävässä pyydetään palata takaisin kohtaan 1:
            continue;
        }

        printf("Yhteys saatu LORA:an!\n");



        //LUETAAN FIRMWARE
        send_cmd("AT+VER");

        if (read_line(line, ROW_LEN,500)) {
            printf("Firmware: %s\n", line);
        }
        else {
            printf("Moduuli lopetti toimintansa.\n");
            continue;
        }

        //Mahdollinen "OK"
        read_line(line, ROW_LEN, 500);




        //DevEUI ja vielä muokataan se LoRaWAN-muotoon
        send_cmd("AT+ID=DevEui");

        if (!read_line(line, ROW_LEN, 500)) {
            printf("Moduuli lopetti toimintansa.\n");
            continue;
        }

        printf("DevEui: %s\n", line);

        //Etsitään pilkku, jonka jälkeen saadaan varsinainen EUI
        char *COMMA = strchr(line, ',');

        if (COMMA) {
            format_eui(cleaned,COMMA + 1 );
            printf("Muokattu DevEui; %s\n\n", cleaned);
        }
        else {
            printf("Ei saatu DevEui.\n\n");
        }

        //Mahdollinen "OK"
        read_line(line,ROW_LEN,500);
    }

}

/*Luetaan yksi rivi kerrallaan:
Jos TRUE = onnistunut:
Jos FALSE = timeout:
*/

bool read_line(char *buffer, int max, int timeout_ms) {
    int i = 0;
    absolute_time_t TIME_LIMIT = make_timeout_time_ms(timeout_ms);

    while (1) {

        //Data on luettavissa UARTista
        if (uart_is_readable(LORA)) {
            char c = uart_getc(LORA);

            //Rivinvaihto:
            if (c == '\n' || c == '\r') {
                if (i > 0) {
                    buffer[i] = '\0';
                    return true;
                }
            }
            else {
                if (i < max -1) {
                    buffer[i++] = c;
                }
            }
        }

        if (absolute_time_diff_us(get_absolute_time(), TIME_LIMIT) <= 0) {
            buffer[i] = '\0';
            return false;
        }
    }
}

//Lähetetään AT komento:
void send_cmd(const char *cmd) {
    uart_puts(LORA, cmd);
    uart_puts(LORA, "\r\n");
}


//Muutetaan kirajaimet pieniksi ja poistetaan kaksoispisteet:
void format_eui(char *output, const char *input) {
    int i = 0;
    int j = 0;

    for (i = 0; input[i]; i++) {

        if (input[i] != ':') {
            output[j++] = (char)tolower((unsigned char)input[i]);
        }
        output[j] = '\0';
    }
}