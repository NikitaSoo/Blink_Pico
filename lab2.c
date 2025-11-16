#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/util/queue.h"

//Maximi kirkkaus:
#define MAX_BRIGHTNESS 1000

//LED pinnit:
#define LED_PIN1 20
#define LED_PIN2 21
#define LED_PIN3 22

//Rotary encoder pinnit:
#define ROT_A 10
#define ROT_B 11
#define ROT_SW 12


//ISR laittaa muistiin käännökset:
queue_t rot_queue;

void gpio_interrupt(uint gpio, uint32_t events);

int main() {

    //"Rotary encoder" asetukset:
    gpio_init(ROT_A);
    gpio_set_dir(ROT_A, GPIO_IN);
    gpio_disable_pulls(ROT_A);

    gpio_init(ROT_B);
    gpio_set_dir(ROT_B, GPIO_IN);
    gpio_disable_pulls(ROT_B);


    //NAPPI:
    gpio_init(ROT_SW);
    gpio_set_dir(ROT_SW, GPIO_IN);
    gpio_pull_up(ROT_SW);


    //LED-pinnit PWM-käyttöön:
    gpio_set_function(LED_PIN1, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN2, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN3, GPIO_FUNC_PWM);


    //"Slices":
    uint slice1 = pwm_gpio_to_slice_num(LED_PIN1);
    uint slice2 = pwm_gpio_to_slice_num(LED_PIN2);
    uint slice3 = pwm_gpio_to_slice_num(LED_PIN3);


    //Jaetaan 125 MHz 125:llä, josta tulee = 1 MHz:
    pwm_set_clkdiv(slice1, 125.0f);
    pwm_set_clkdiv(slice2, 125.0f);
    pwm_set_clkdiv(slice3, 125.0f);

    //1 kHz (0-999 arvot):
    pwm_set_wrap(slice1, MAX_BRIGHTNESS - 1);
    pwm_set_wrap(slice2, MAX_BRIGHTNESS - 1);
    pwm_set_wrap(slice3, MAX_BRIGHTNESS - 1);

    //Käynnistetään PWM:
    pwm_set_enabled(slice1, true);
    pwm_set_enabled(slice2, true);
    pwm_set_enabled(slice3, true);


    //Jonon alustus:
    queue_init(&rot_queue, sizeof(int), 32);

    gpio_set_irq_enabled_with_callback(ROT_A,
        GPIO_IRQ_EDGE_RISE,
        true,
        &gpio_interrupt
        );

    //(Asetetaan alkuarvo kirkkaudelle 50%):
    int brightness = 500;
    bool LEDS_ON = false; //muuttuja joka pitää huolen ovatko LEDit päällä
    bool prev_button = true; //tämä tallentaa napin aiemman tilan

    //LOOP
    while (1) {
        bool current_button = gpio_get(ROT_SW);

        //Tarkistetaan painallus
        if (!current_button && prev_button) {
            sleep_ms(200); //"debounce"

            //Jos kirkkaus 0 niin palautetaan 50%
            if (LEDS_ON && brightness == 0) {
                brightness = 500;
            }

            //Vaihdetaan LEDien tila päälle tai pois.
            else {
                LEDS_ON = !LEDS_ON;
                if (LEDS_ON && brightness == 0) {
                    brightness = 500;
                }
            }
        }

        //Tallentaa napin nykyisen tilan:
        prev_button = current_button;

        //Encoderin käsittely jonosta
        int direction;

        while (queue_try_remove(&rot_queue, &direction)) {
            if (LEDS_ON) {
                brightness += direction * 20; //+1 kirkkaampi, -1 himmeämpi
            }

            if (brightness < 0) brightness = 0;
            if (brightness > MAX_BRIGHTNESS) brightness = MAX_BRIGHTNESS;
        }

        //Päivitetään LEDien tila:
        if (LEDS_ON) {
            pwm_set_gpio_level(LED_PIN1, brightness);
            pwm_set_gpio_level(LED_PIN2, brightness);
            pwm_set_gpio_level(LED_PIN3, brightness);
        }
        else {
            pwm_set_gpio_level(LED_PIN1, 0);
            pwm_set_gpio_level(LED_PIN2, 0);
            pwm_set_gpio_level(LED_PIN3, 0);
        }

        sleep_ms(5);
    }
}

//Keskeytysfunktio:
void gpio_interrupt(uint gpio, uint32_t events) {
    if (gpio == ROT_A) {
        if (events & GPIO_IRQ_EDGE_RISE) {
            int direction;

            //Myötäpäivään:
            if (gpio_get(ROT_B))
                direction = 1;

            //Vastapäivään:
            else
                direction = -1;

            //Lisätään tapahtuma jonoon:
            queue_try_add(&rot_queue, &direction);
        }
    }
}