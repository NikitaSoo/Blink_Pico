#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define MAX_BRIGHTNESS 1000
#define MIN_BRIGHTNESS 0

#define ROT_A 10
#define ROT_B 11
#define ROT_SW 12

volatile bool rotated = false;
volatile int direction = 0; // +1 or -1

// --- Interrupt for the rotary encoder ---
void gpio_handler(uint gpio, uint32_t event_mask) {
    bool b_state = gpio_get(ROT_B);
    if (b_state)
        direction = 1;   // Clockwise
    else
        direction = -1;  // Counter-clockwise
    rotated = true;
}

int main() {
    stdio_init_all();

    // LED pins
    const uint LED_PIN1 = 20;
    const uint LED_PIN2 = 21;
    const uint LED_PIN3 = 22;

    //Set pins to PWM function
    gpio_set_function(LED_PIN1, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN2, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN3, GPIO_FUNC_PWM);

    //Get PWM slices and channels
    uint slice_num1 = pwm_gpio_to_slice_num(LED_PIN1);
    uint channel1   = pwm_gpio_to_channel(LED_PIN1);

    uint slice_num2 = pwm_gpio_to_slice_num(LED_PIN2);
    uint channel2   = pwm_gpio_to_channel(LED_PIN2);

    uint slice_num3 = pwm_gpio_to_slice_num(LED_PIN3);
    uint channel3   = pwm_gpio_to_channel(LED_PIN3);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 125);   // 1 MHz base clock
    pwm_config_set_wrap(&config, 999);         // 1 kHz PWM
    pwm_init(slice_num1, &config, false);
    pwm_init(slice_num2, &config, false);
    pwm_init(slice_num3, &config, false);

    uint brightness = 500;       // current level
    uint last_brightness = 500;  // remembered level
    bool leds_on = true;
    bool prev_button = false;

    pwm_set_chan_level(slice_num1, channel1, brightness);
    pwm_set_chan_level(slice_num2, channel2, brightness);
    pwm_set_chan_level(slice_num3, channel3, brightness);
    pwm_set_enabled(slice_num1, true);
    pwm_set_enabled(slice_num2, true);
    pwm_set_enabled(slice_num3, true);

    // --- Rotary encoder setup ---
    gpio_init(ROT_A);
    gpio_set_dir(ROT_A, GPIO_IN);
    gpio_init(ROT_B);
    gpio_set_dir(ROT_B, GPIO_IN);
    gpio_init(ROT_SW);
    gpio_set_dir(ROT_SW, GPIO_IN);
    gpio_pull_up(ROT_SW);

    gpio_set_irq_enabled_with_callback(ROT_A, GPIO_IRQ_EDGE_RISE, true, &gpio_handler);

    printf("Rotary encoder interrupt setup done.\n");

    while (1) {
        // --- Handle rotation ---
        if (rotated) {
            rotated = false;

            if (leds_on) {
                if (direction > 0 && brightness < MAX_BRIGHTNESS - 10)
                    brightness += 10;
                else if (direction < 0 && brightness > MIN_BRIGHTNESS + 10)
                    brightness -= 10;
                else if (direction < 0 && brightness <= 10)
                    brightness = 0; // full off at bottom

                if (brightness > 0)
                    last_brightness = brightness; // remember level

                pwm_set_chan_level(slice_num1, channel1, brightness);
                pwm_set_chan_level(slice_num2, channel2, brightness);
                pwm_set_chan_level(slice_num3, channel3, brightness);
                printf("Brightness: %u\n", brightness);
            }
        }

        // --- Handle push button (toggle on/off) ---
        bool button_pressed = !gpio_get(ROT_SW);

        if (button_pressed && !prev_button) {
            if (leds_on) {
                // if LEDs are ON and at 0 %, set 50 % immediately
                if (brightness == 0) {
                    brightness = 500;
                    last_brightness = 500;
                } else {
                    // turn off, but remember brightness
                    last_brightness = brightness;
                    brightness = 0;
                    leds_on = false;
                }
            } else {
                // turn ON → restore remembered brightness
                leds_on = true;
                if (last_brightness == 0)
                    brightness = 500; // default 50 %
                else
                    brightness = last_brightness;
            }

            pwm_set_chan_level(slice_num1, channel1, brightness);
            pwm_set_chan_level(slice_num2, channel2, brightness);
            pwm_set_chan_level(slice_num3, channel3, brightness);
        }

        prev_button = button_pressed;
        sleep_ms(5);
    }
}