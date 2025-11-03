#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"


#define MAX_BRIGHTNESS 1000
#define MIN_BRIGHTNESS 0


int main() {
    //function to use printf in code :)
    stdio_init_all();

    //LED pins
    const uint LED_PIN1 = 20;
    const uint LED_PIN2 = 21;
    const uint LED_PIN3 = 22;

    //Buttons:
    //SW0 button will decrease brightness
    //SW1 is ON/OFF button (middle)
    //SW2 button will increase brightness
    const uint SW0 = 7;
    const uint SW1 = 8;
    const uint SW2 = 9;


    /*Kaikki mitä tossa alhaalla on tehty ohjeiden mukaisesti, jotka löytyivät
    02_PWM.pdf main looppiin asti -->
     */

    //Set pins to PWM function
    gpio_set_function(LED_PIN1, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN2, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN3, GPIO_FUNC_PWM);

    //Get PWM slices and channels
    uint slice_num1 = pwm_gpio_to_slice_num(LED_PIN1);
    uint channel1 = pwm_gpio_to_channel(LED_PIN1);

    uint slice_num2 = pwm_gpio_to_slice_num(LED_PIN2);
    uint channel2 = pwm_gpio_to_channel(LED_PIN2);

    uint slice_num3 = pwm_gpio_to_slice_num(LED_PIN3);
    uint channel3 = pwm_gpio_to_channel(LED_PIN3);

    //Print the LED pins GPIO
    printf("LED_PIN1: GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN1, slice_num1, channel1);
    printf("LED_PIN2: GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN2, slice_num2, channel2);
    printf("LED_PIN3: GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN3, slice_num3, channel3);

    //Stop PWM
    pwm_set_enabled(slice_num1, false);
    pwm_set_enabled(slice_num2, false);
    pwm_set_enabled(slice_num3, false);

    //Just to get deafult config
    pwm_config config = pwm_get_default_config();

    /*Divider is configured to output 1 MHz (the RP2040 runs on the main clock 125MHz)
     *Thats way too fast so we divide that clock by 125 (125 MHz / 125 = 1MHz)
    */
    pwm_config_set_clkdiv_int(&config, 125);

    //1kHz PWM output (so they counter goes from 0-999) total steps per cycle
    pwm_config_set_wrap(&config, 999);

    //Initialize PWM with this config (dont start yet)
    pwm_init(slice_num1, &config, false);
    pwm_init(slice_num2, &config, false);
    pwm_init(slice_num3, &config, false);


    //Set brightness
    uint brightness = 500; //asetetaan alkuarvo kirkkaudelle 50%
    uint last_brightness = 500; //!!

    bool LEDS_ON = true; //muuttuja joka pitää huolen ovatko LEDit päällä
    bool previous_sw1 = false; //tämä tallentaa napin aiemman tilan

    //Set brightness (duty cycle)
    pwm_set_chan_level(slice_num1, channel1, brightness);
    pwm_set_chan_level(slice_num2, channel2, brightness);
    pwm_set_chan_level(slice_num3, channel3, brightness);

    //Start PWM output
    pwm_set_enabled(slice_num1, true);
    pwm_set_enabled(slice_num2, true);
    pwm_set_enabled(slice_num3, true);

    //Setup buttons (every button is made as input
    gpio_init(SW0); gpio_set_dir(SW0, GPIO_IN); gpio_pull_up(SW0);
    gpio_init(SW1); gpio_set_dir(SW1, GPIO_IN); gpio_pull_up(SW1);
    gpio_init(SW2); gpio_set_dir(SW2, GPIO_IN); gpio_pull_up(SW2);


    //Main loop

    //"true" when button is pressed "false" when released
    while (1) {
        bool sw0 = !gpio_get(SW0); //decrease
        bool sw1 = !gpio_get(SW1); //toggle
        bool sw2 = !gpio_get(SW2); //increase

        //SW1 toggle
        if (sw1 && !previous_sw1) {
            if (LEDS_ON) {

                if (brightness == 0) {
                    //If already fully dimmed, go straight to 50% instead of off
                    brightness = 500;
                    last_brightness = 500;
                }

                else {
                    //Turn LEDs OFF and remember last brightness
                    LEDS_ON = false;

                    if (brightness > 0) {
                        last_brightness = brightness;
                    }
                    else {
                        last_brightness = 0;
                    }

                    brightness = 0;
                }
            }

            else {
                //Turn LEDs ON
                LEDS_ON = true;

                if (last_brightness > 0) {
                    brightness = last_brightness;
                }
                else {
                    brightness = 500;
                }
            }
        }
        previous_sw1 = sw1;

        //We adjust brightness only if LEDs are ON
        if (LEDS_ON) {

            //Decrease brightness
            if (sw0 && brightness > 0) {
                brightness--;

                if (brightness == 0) {
                    last_brightness = 0;
                    LEDS_ON = false;
                }
            }

            //Increase brightness
            if (sw2 && brightness < MAX_BRIGHTNESS) {
                brightness++;

                if (brightness > 0) {
                    last_brightness = brightness;
                }
            }
        }

        //And then we update all LEDs
        pwm_set_chan_level(slice_num1, channel1, brightness);
        pwm_set_chan_level(slice_num2, channel2, brightness);
        pwm_set_chan_level(slice_num3, channel3, brightness);

        //Use sleep to make dimming smooth (if we dont use sleep then it's not quite a dimming) :)
        //Without sleep when we press buttons they will just put lowest and highest values
        sleep_ms(10);
    }
}