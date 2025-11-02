#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

int main() {
    stdio_init_all();

    // LED pins
    const uint LED_PIN1 = 20;
    const uint LED_PIN2 = 21;
    const uint LED_PIN3 = 22;

    // Buttons:
    // SW0 button will decrease brightness
    // SW1 is ON/OFF button (middle)
    // SW2 button will increase brightness
    const uint SW0 = 7;
    const uint SW1 = 8;
    const uint SW2 = 9;

    // Set pins to PWM function
    gpio_set_function(LED_PIN1, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN2, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN3, GPIO_FUNC_PWM);
    printf("GPIO pins set to PWM function.\n");

    // Get PWM slices and channels
    uint slice_num1 = pwm_gpio_to_slice_num(LED_PIN1);
    uint channel1 = pwm_gpio_to_channel(LED_PIN1);

    uint slice_num2 = pwm_gpio_to_slice_num(LED_PIN2);
    uint channel2 = pwm_gpio_to_channel(LED_PIN2);

    uint slice_num3 = pwm_gpio_to_slice_num(LED_PIN3);
    uint channel3 = pwm_gpio_to_channel(LED_PIN3);

    // Print the LED pins GPIO, slice and channel
    printf("LED_PIN1: GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN1, slice_num1, channel1);
    printf("LED_PIN2: GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN2, slice_num2, channel2);
    printf("LED_PIN3: GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN3, slice_num3, channel3);

    // Stop PWM
    pwm_set_enabled(slice_num1, false);
    pwm_set_enabled(slice_num2, false);
    pwm_set_enabled(slice_num3, false);

    // Get default config and set parameters
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 125);  // 1 MHz base clock
    pwm_config_set_wrap(&config, 999);        // 1 kHz PWM output

    // Initialize PWM with this config (don't start yet)
    pwm_init(slice_num1, &config, false);
    pwm_init(slice_num2, &config, false);
    pwm_init(slice_num3, &config, false);

    uint brightness = 500;
    bool LEDS_ON = true;
    bool previous_sw1 = false;

    // Set brightness (duty cycle)
    pwm_set_chan_level(slice_num1, channel1, brightness);
    pwm_set_chan_level(slice_num2, channel2, brightness);
    pwm_set_chan_level(slice_num3, channel3, brightness);
    printf("PWM channel levels (duty cycles) set.\n");

    // Start PWM output
    pwm_set_enabled(slice_num1, true);
    pwm_set_enabled(slice_num2, true);
    pwm_set_enabled(slice_num3, true);
    printf("PWM started for all LED slices.\n");

    // Setup buttons
    gpio_init(SW0); gpio_set_dir(SW0, GPIO_IN); gpio_pull_up(SW0);
    gpio_init(SW1); gpio_set_dir(SW1, GPIO_IN); gpio_pull_up(SW1);
    gpio_init(SW2); gpio_set_dir(SW2, GPIO_IN); gpio_pull_up(SW2);

    while (1) {
        bool sw0 = !gpio_get(SW0); // decrease
        bool sw1 = !gpio_get(SW1); // toggle
        bool sw2 = !gpio_get(SW2); // increase

        //SW1 toggle
        if (sw1 && !previous_sw1) {
            LEDS_ON = !LEDS_ON;
            if (!LEDS_ON) brightness = 0;
            else if (brightness == 0) brightness = 500; // restore to 50% if off
        }
        previous_sw1 = sw1;

        //Adjust brightness only if LEDs ON
        if (LEDS_ON) {
            if (sw0 && brightness > 0) brightness--;
            if (sw2 && brightness < 999) brightness++;
        }

        //Update all LEDs
        pwm_set_chan_level(slice_num1, channel1, brightness);
        pwm_set_chan_level(slice_num2, channel2, brightness);
        pwm_set_chan_level(slice_num3, channel3, brightness);

        sleep_ms(5);// smooth dimming
    }
}