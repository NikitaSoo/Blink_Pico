#include <stdio.h>
#include "pico/stdlib.h"

// Stepper pins
#define IN1 2
#define IN2 3
#define IN3 6
#define IN4 13

// Opto sensor
#define OPTO 28

// Half-step sequence
const uint8_t step_table[8][4] = {
        {1,0,0,0},
        {1,1,0,0},
        {0,1,0,0},
        {0,1,1,0},
        {0,0,1,0},
        {0,0,1,1},
        {0,0,0,1},
        {1,0,0,1}
};

int step_index = 0;
bool calibrated = false;
int steps_per_rev = 0;

void set_step(int idx) {
    gpio_put(IN1, step_table[idx][0]);
    gpio_put(IN2, step_table[idx][1]);
    gpio_put(IN3, step_table[idx][2]);
    gpio_put(IN4, step_table[idx][3]);
}

void step_forward() {
    step_index = (step_index + 1) & 7;
    set_step(step_index);
    sleep_ms(2);
}

void step_backward() {
    step_index = (step_index + 7) & 7;
    set_step(step_index);
    sleep_ms(2);
}

bool falling_edge_seen() {
    static bool last = true;
    bool now = gpio_get(OPTO);
    bool fall = (last == true && now == false);
    last = now;
    return fall;
}

int calibrate_once() {
    // Move until falling edge
    while (!falling_edge_seen())
        step_forward();

    // Now count one full revolution
    int count = 0;
    while (!falling_edge_seen()) {
        step_forward();
        count++;
    }
    return count;
}

void do_calibration() {
    printf("Calibrating...\n");

    int a = calibrate_once();
    int b = calibrate_once();
    int c = calibrate_once();

    steps_per_rev = (a + b + c) / 3;
    calibrated = true;

    printf("Calibration done. Steps/rev = %d\n", steps_per_rev);
}

void run_steps(int n) {
    if (!calibrated) {
        printf("Not calibrated!\n");
        return;
    }

    int steps = (steps_per_rev / 8) * n;
    for (int i = 0; i < steps; i++)
        step_forward();

    printf("Motor run %d/8 turns.\n", n);
}

int main() {
    stdio_init_all();

    gpio_init(IN1); gpio_set_dir(IN1, GPIO_OUT);
    gpio_init(IN2); gpio_set_dir(IN2, GPIO_OUT);
    gpio_init(IN3); gpio_set_dir(IN3, GPIO_OUT);
    gpio_init(IN4); gpio_set_dir(IN4, GPIO_OUT);

    gpio_init(OPTO); gpio_set_dir(OPTO, GPIO_IN);
    gpio_pull_up(OPTO);

    printf("Stepper motor ready.\n");

    char cmd[32];
    while (true) {
        printf("> ");
        scanf("%s", cmd);

        if (strcmp(cmd, "status") == 0) {
            printf("Calibrated: %s\n", calibrated ? "yes" : "no");
            if (calibrated)
                printf("Steps/rev: %d\n", steps_per_rev);
            else
                printf("Steps/rev: not available\n");
        }

        else if (strcmp(cmd, "calib") == 0) {
            do_calibration();
        }

        else if (strcmp(cmd, "run") == 0) {
            int n = 8; // full turn default
            scanf("%d", &n);
            run_steps(n);
        }

        else {
            printf("Unknown command\n");
        }
    }
}
