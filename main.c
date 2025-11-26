#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

//Moottori pinnit:
#define IN1 2
#define IN2 3
#define IN3 6
#define IN4 13

//Optohaarukka:
#define OPTO 28


//Funktiot joita käytetään:
void step_once();
void falling_edge();
int measure_one_rev();
void calibrate();
void run_motor(int N);


//Half-step sekvenssi (8 vaiheitta)
int step_table[8][4] = {
    {1,0,0,0}, //1
    {1,1,0,0}, //2
    {0,1,0,0}, //3
    {0,1,1,0}, //4
    {0,0,1,0}, //5
    {0,0,1,1}, //6
    {0,0,0,1}, //7
    {1,0,0,1}, //8
};

int STEP_index = 0;
int STEPS_rev = 0;
int calibrated = 0;

int main() {
    stdio_init_all();

    //Moottorin alustus:
    gpio_init(IN1); gpio_set_dir(IN1, GPIO_OUT);
    gpio_init(IN2); gpio_set_dir(IN2, GPIO_OUT);
    gpio_init(IN3); gpio_set_dir(IN3, GPIO_OUT);
    gpio_init(IN4); gpio_set_dir(IN4, GPIO_OUT);

    //Optohaarukan alustus:
    gpio_init(OPTO);
    gpio_set_dir(OPTO, GPIO_IN);
    gpio_pull_up(OPTO);

    char cmd[32];

    while (1) {

        //Luetaan komento USB-sarjaportista:
        if (!fgets(cmd, sizeof(cmd),stdin)) {
            continue;
        }

        //"Status" komento:
        if (strncmp(cmd, "status",6) == 0) {
            if (calibrated == 1) {
                printf("Kalibroitu: YES\n");
                printf("Vaiheet per kierros: %d\n", STEPS_rev);
            }
            else {
                printf("Kalibroitu: NO\n");
            }
        }

        //Calib-komento
        else if (strncmp(cmd, "calib", 5) == 0) {
            calibrate();
        }

        //RUN-komento:
        else if (strncmp(cmd, "run", 3) == 0) {
            int N;
            if (sscanf(cmd, "run %d", &N) == 1) {

                //suoritetaan "N" kertaa
                run_motor(N);

            }

            //Suoritetaan yksi täysi kierros
            else {
                run_motor(-1);
            }
        }
        else {
            printf("Virheellinen komento, komennot ovat: status | calib | run[N]\n");
        }
    }
}

//yksi vaihe myötäpäivään
void step_once() {
    STEP_index = (STEP_index + 1) % 8;
    gpio_put(IN1, step_table[STEP_index][0]);
    gpio_put(IN2, step_table[STEP_index][1]);
    gpio_put(IN3, step_table[STEP_index][2]);
    gpio_put(IN4, step_table[STEP_index][3]);

    sleep_ms(3);
}

//odotetaan laskevaa reunaa
void falling_edge() {
    int prev = gpio_get(OPTO);

    while (true) {
        step_once();
        int now = gpio_get(OPTO);
        if (prev == 1 && now == 0) {
            return;
        }
        prev = now;
    }
}

//lasketaan vaiheiden määrä yhden kierroksen aikana
int measure_one_rev() {
    int prev = gpio_get(OPTO);
    int count = 0;

    while (true) {
        step_once();
        count++;
        int now = gpio_get(OPTO);
        if (prev == 1 && now == 0) {
            return count;
        }
        prev = now;
    }
}

//kalibrointi:
void calibrate() {
    printf("Kalibroidaan..\n");

    //Etsitään laskeva reuna:
    falling_edge();

    //Kolme kierrosta:
    int FIRST = measure_one_rev();
    int SECOND = measure_one_rev();
    int THIRD = measure_one_rev();

    STEPS_rev = (FIRST + SECOND + THIRD) / 3;
    calibrated = 1;

    printf("Kalibrointi suoritettu. Vaiheet per kierros = %d\n", STEPS_rev);
}

//(N / 8 kierrosta) tai yhden täyden kierroksen, jos N = -1:
void run_motor (int N) {

    int SPR;

    //Käytetään mitattua vaiheiden määrää:
    if (calibrated == 1) {
        SPR = STEPS_rev;
    }

    //Oletusarvo 4096:
    else {
        SPR = 4096;
    }

    //Lasketaan ajettavien askelten määrä:
    int steps;

    if (N == -1) {
        steps = SPR;
    }
    else {
        if (N % 8 == 0) {
            steps = SPR;
        }
        else {
            steps = (SPR / 8) * (N % 8);
        }
    }

    //Suoritetaan:
    for (int i = 0; i < steps; i++) {
        step_once();
    }

    printf("Suoritettu!\n");
}