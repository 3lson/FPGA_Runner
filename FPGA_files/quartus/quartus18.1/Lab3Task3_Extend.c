#include "system.h"
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include <stdlib.h>
#include <stdio.h>

#define OFFSET -32
#define PWM_PERIOD 16
#define TAPS 51  // Update TAPS to match the number of coefficients

alt_8 pwm = 0;
alt_u8 led;
int level;

void led_write(alt_u8 led_pattern) {
    IOWR(LED_BASE, 0, led_pattern);
}

void convert_read(alt_32 acc_read, int * level, alt_u8 * led) {
    acc_read += OFFSET;
    alt_u8 val = (acc_read >> 6) & 0x07;
    *led = (8 >> val) | (8 << (8 - val));
    *level = (acc_read >> 1) & 0x1f;
}

// Hardcoded FIR filter coefficients (as provided)
float coeffs[TAPS] = {
    0.00464135, 0.00737747, -0.00240769, -0.00711019, 0.00326565,
    0.00006115, -0.00935762, 0.00397493, 0.00437887, -0.01331607,
    0.00304772, 0.01143620, -0.01792870, -0.00107408, 0.02225979,
    -0.02247727, -0.01087445, 0.03959728, -0.02632217, -0.03375703,
    0.07519872, -0.02889782, -0.12035485, 0.28792197, 0.63686339,
    0.28792197, -0.12035485, -0.02889782, 0.07519872, -0.03375703,
    -0.02632217, 0.03959728, -0.01087445, -0.02247727, 0.02225979,
    -0.00107408, -0.01792870, 0.01143620, 0.00304772, -0.01331607,
    0.00437887, 0.00397493, -0.00935762, 0.00006115, 0.00326565,
    -0.00711019, -0.00240769, 0.00737747, 0.00464135
};

// Low-pass filter function (FIR)
float LPF(alt_32 acc_read[TAPS], float coeffs[TAPS]) {
    float avg = 0;
    for (int i = 0; i < TAPS; i++) {
        avg += acc_read[i] * coeffs[i];  // FIR filter calculation
    }
    return avg;  // Return the filtered result
}

void sys_timer_isr() {
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);

    if (pwm < abs(level)) {
        if (level < 0) {
            led_write(led << 1);
        } else {
            led_write(led >> 1);
        }
    } else {
        led_write(led);
    }

    if (pwm > PWM_PERIOD) {
        pwm = 0;
    } else {
        pwm++;
    }
}

void timer_init(void * isr) {
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0003);
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_BASE, 0x0900);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_BASE, 0x0000);
    alt_irq_register(TIMER_IRQ, 0, isr);
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0007);
}

int main() {
    int i = 0;
    alt_32 x_read[TAPS];
    // Initialize x_read to zero
    for (int j = 0; j < TAPS; j++) {
        x_read[j] = 0;
    }

    alt_up_accelerometer_spi_dev * acc_dev;
    acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
    if (acc_dev == NULL) {  // if return 1, check if the SPI device name is "accelerometer_spi"
        return 1;
    }

    timer_init(sys_timer_isr);

    while (1) {
        if (i == TAPS) {
            i = 0;  // Reset index once we reach TAPS
        }

        alt_up_accelerometer_spi_read_x_axis(acc_dev, &x_read[i]);
        float avg = LPF(x_read, coeffs);  // Apply FIR filter

        // Map the filtered result to the LED pattern
        convert_read(avg, &level, &led);

        i++;
    }

    return 0;
}
