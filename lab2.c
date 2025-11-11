#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

//Maximi LED valojen kirkkaus ja minimi
#define MAX_BRIGHTNESS 1000
#define MIN_BRIGHTNESS 0

//Rotary encoder pinnit:
#define ROT_A 10
#define ROT_B 11
#define ROT_SW 12 //kytkin (kun painaa siitä)

//LED pinnit:
#define LED_PIN1 20
#define LED_PIN2 21
#define LED_PIN3 22


//Globaalit muuttujat ("volatile" on käytännössä se, että muuttujan arvo voi muuttua)
volatile bool rotated = false;
volatile int direction = 0; // +1 tai -1




/*Funktio suoritetaan aina Encoder havaitsee reunan (nousevan tai laskevan)
  Funktio ei muuta PWM-arvoa suoraan. Sen sijaan ilmoittaa siitä "Mainin" sisällä */

static void gpio_handler(uint gpio, uint32_t event_mask) {
    bool b_state = gpio_get(ROT_B);

    //Myötäpäivää
    if (b_state)
        direction = 1;

    //Vastapäivää
    else
        direction = -1;

    rotated = true; //ilmoitetaan "mainin" sisällä, että kääntö tapahtui
}

int main() {

    //Printf toimii :)
    stdio_init_all();

    //LED-pinnit PWM-käyttöön
    gpio_set_function(LED_PIN1, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN2, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN3, GPIO_FUNC_PWM);


    //"Slices" ja "channels"
    uint slice_num1 = pwm_gpio_to_slice_num(LED_PIN1);
    uint channel1 = pwm_gpio_to_channel(LED_PIN1);

    uint slice_num2 = pwm_gpio_to_slice_num(LED_PIN2);
    uint channel2 = pwm_gpio_to_channel(LED_PIN2);

    uint slice_num3 = pwm_gpio_to_slice_num(LED_PIN3);
    uint channel3 = pwm_gpio_to_channel(LED_PIN3);


    //"Default config"
    pwm_config config = pwm_get_default_config();

    //Jaetaan 125 MHz 125:llä, josta tulee = 1 MHz
    pwm_config_set_clkdiv_int(&config, 125);

    //1kHz PWM output (so they counter goes from 0-999) total steps per cycle
    pwm_config_set_wrap(&config, 999);

    //Initialize PWM with this config (dont start yet)
    pwm_init(slice_num1, &config, false);
    pwm_init(slice_num2, &config, false);
    pwm_init(slice_num3, &config, false);


    //(Asetetaan alkuarvo kirkkaudelle 50%)
    uint brightness = 500;
    bool leds_on = true; //muuttuja joka pitää huolen ovatko LEDit päällä
    bool prev_button = false; //tämä tallentaa napin aiemman tilan

    //Asetetaan PWM-kanaville alkuarvot
    pwm_set_chan_level(slice_num1, channel1, brightness);
    pwm_set_chan_level(slice_num2, channel2, brightness);
    pwm_set_chan_level(slice_num3, channel3, brightness);

    //Käynnistetään PWM
    pwm_set_enabled(slice_num1, true);
    pwm_set_enabled(slice_num2, true);
    pwm_set_enabled(slice_num3, true);



    //"Rotary encoder" asetukset
    gpio_init(ROT_A);
    gpio_set_dir(ROT_A, GPIO_IN);

    gpio_init(ROT_B);
    gpio_set_dir(ROT_B, GPIO_IN);

    gpio_init(ROT_SW);
    gpio_set_dir(ROT_SW, GPIO_IN);
    gpio_pull_up(ROT_SW); //Encoder nappi, tarvitaan siihen pull-up



    //(ISR) keskeytysfunktio, kun ROT_A havaitsee reunan suoritetaan gpio_handler funktio
    gpio_set_irq_enabled_with_callback(
        ROT_A,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        &gpio_handler);


    //Loop
    while (1) {
        // --- Handle rotation ---
        if (rotated) {
            rotated = false;  // clear flag

            if (leds_on) {
                if (direction > 0 && brightness < MAX_BRIGHTNESS)
                    brightness += 5;
                else if (direction < 0 && brightness > MIN_BRIGHTNESS)
                    brightness -= 5;

                pwm_set_chan_level(slice_num1, channel1, brightness);
                pwm_set_chan_level(slice_num2, channel2, brightness);
                pwm_set_chan_level(slice_num3, channel3, brightness);
                printf("Brightness: %u\n", brightness);
            }
        }

        // --- Handle push button (toggle on/off) ---
        bool button_pressed = !gpio_get(ROT_SW);

        if (button_pressed && !prev_button) {
            leds_on = !leds_on;

            if (!leds_on) {
                brightness = 0;
            } else if (brightness == 0) {
                brightness = 500; // restore to 50%
            }

            pwm_set_chan_level(slice_num1, channel1, brightness);
            pwm_set_chan_level(slice_num2, channel2, brightness);
            pwm_set_chan_level(slice_num3, channel3, brightness);
        }

        prev_button = button_pressed;
        sleep_ms(10);
    }
}