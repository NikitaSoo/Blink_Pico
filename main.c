#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

int main() {
    stdio_init_all();

    const uint LED_PIN1 = 20;
    const uint LED_PIN2 = 21;
    const uint LED_PIN3 = 22;

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

    printf("LED_PIN1: GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN1, slice_num1, channel1);
    printf("LED_PIN2: GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN2, slice_num2, channel2);
    printf("LED_PIN3: GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN3, slice_num3, channel3);

    // Stop PWM initially
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

    // Set brightness (duty cycle)
    pwm_set_chan_level(slice_num1, channel1, 500);  // ~50%
    pwm_set_chan_level(slice_num2, channel2, 750);  // ~75%
    pwm_set_chan_level(slice_num3, channel3, 250);  // ~25%
    printf("PWM channel levels (duty cycles) set.\n");

    // ✅ Start PWM output
    pwm_set_enabled(slice_num1, true);
    pwm_set_enabled(slice_num2, true);
    pwm_set_enabled(slice_num3, true);
    printf("PWM started for all LED slices.\n");

    // --- Example: adjust duty cycle while running ---
    // Increase LED1 to 80%, LED2 to 50%, LED3 to 100%
    pwm_set_chan_level(slice_num1, channel1, 800);
    pwm_set_chan_level(slice_num2, channel2, 500);
    pwm_set_chan_level(slice_num3, channel3, 999);

    while (1) {
        // Keep running forever
    }
}
