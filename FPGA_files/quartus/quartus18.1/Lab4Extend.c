#include <stdio.h>
#include <string.h>
#include "system.h"
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_stdio.h"
#include "altera_avalon_jtag_uart_regs.h"

#define OFFSET -32
#define PWM_PERIOD 16
#define TAPS 49  // Update TAPS to match the number of coefficients

volatile int mode = 0; // 0: unfiltered, 1: filtered
alt_up_accelerometer_spi_dev *acc_dev;
alt_8 pwm = 0;
alt_u8 led;
int level;

// Hardcoded FIR filter coefficients
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

// Function prototypes
void timer_isr(void *context, alt_u32 id);
float LPF(alt_32 acc_read[TAPS]);
void update_leds(float value);
void uart_isr(void *context, alt_u32 id);
void convert_read(alt_32 acc_read, int *level, alt_u8 *led);

int main() {
    // Initialize accelerometer
    acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
    if (acc_dev == NULL) {
        printf("Error: Could not open accelerometer device\n");
        return 1;
    }

    // Initialize timer
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0003);
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_BASE, 0x0900);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_BASE, 0x0000);
    alt_irq_register(TIMER_IRQ, NULL, timer_isr);
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x0007);

    // Initialize UART interrupt
    alt_irq_register(JTAG_UART_IRQ, NULL, uart_isr);
    IOWR_ALTERA_AVALON_JTAG_UART_CONTROL(JTAG_UART_BASE, 0x1);

    printf("System initialized. Waiting for commands...\n");

    while (1) {
        // Main loop is now empty, as processing is done in interrupts
    }

    return 0;
}

void timer_isr(void *context, alt_u32 id) {
    static int i = 0;
    static alt_32 x_read[TAPS] = {0};

    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE, 0);

    if (i == TAPS) {
        i = 0;  // Reset index once we reach TAPS
    }

    alt_up_accelerometer_spi_read_x_axis(acc_dev, &x_read[i]);

    float value;
    if (mode == 1) {
        value = LPF(x_read);
    } else {
        value = x_read[i];
    }

    convert_read((alt_32)value, &level, &led);

    if (pwm < abs(level)) {
        if (level < 0) {
            IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, led << 1);
        } else {
            IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, led >> 1);
        }
    } else {
        IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, led);
    }

    if (pwm > PWM_PERIOD) {
        pwm = 0;
    } else {
        pwm++;
    }

    i++;
}

float LPF(alt_32 acc_read[TAPS]) {
    float avg = 0;
    for (int i = 0; i < TAPS; i++) {
        avg += acc_read[i] * coeffs[i];  // FIR filter calculation
    }
    //Amplify the difference
    avg *= 2.0;
    return avg;  // Return the filtered result
}

void convert_read(alt_32 acc_read, int *level, alt_u8 *led) {
    acc_read += OFFSET;
    alt_u8 val = (acc_read >> 6) & 0x07;
    *led = (8 >> val) | (8 << (8 - val));
    *level = (acc_read >> 1) & 0x1f;
}

void uart_isr(void *context, alt_u32 id) {
    char cmd = IORD_ALTERA_AVALON_JTAG_UART_DATA(JTAG_UART_BASE) & 0xFF;

    if (cmd == '0') {
        mode = 0;
        printf("Mode set to 0 (unfiltered)\n");
    } else if (cmd == '1') {
        mode = 1;
        printf("Mode set to 1 (filtered)\n");
    } else if (cmd == 'r') {
        alt_32 x_read;
        alt_up_accelerometer_spi_read_x_axis(acc_dev, &x_read);
        printf("Accelerometer Data: %ld\n", x_read);
    }
}
