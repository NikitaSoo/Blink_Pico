#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

int main() {
    const uint LED_PIN1 = 20;
    const uint LED_PIN2 = 21;
    const uint LED_PIN3 = 22;

    //PWM slices and channels for pins
    uint slice_num1 = pwm_gpio_to_slice_num(LED_PIN1);
    uint channel1 = pwm_gpio_to_channel(LED_PIN1);

    uint slice_num2 = pwm_gpio_to_slice_num(LED_PIN2);
    uint channel2 = pwm_gpio_to_channel(LED_PIN2);

    uint slice_num3 = pwm_gpio_to_slice_num(LED_PIN3);
    uint channel3 = pwm_gpio_to_channel(LED_PIN3);

    //Print result
    printf("GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN1, slice_num1, channel1);
    printf("GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN2, slice_num2, channel2);
    printf("GPIO: %u, PWM slice: %u, channel: %u\n", LED_PIN3, slice_num3, channel3);

    return 0;
}
