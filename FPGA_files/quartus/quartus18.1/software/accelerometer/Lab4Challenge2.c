#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#define TAPS 49  // Number of filter taps
#define MAX_COEFF_DIGITS 10  // Maximum digits per coefficient
#define COEFF_DELIMITER ','  // Delimiter between coefficients

volatile int mode = 0; // 0: unfiltered, 1: filtered
volatile int continuous_mode = 0;
alt_up_accelerometer_spi_dev *acc_dev;
alt_8 pwm = 0;
alt_u8 led;
int level;

// FIR filter coefficients
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
void convert_read(alt_32 acc_read, int *level, alt_u8 *led);
void uart_isr(void *context, alt_u32 id);

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
        // Main loop is empty, processing is done in interrupts
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
    return avg;  // Return the filtered result
}

void convert_read(alt_32 acc_read, int *level, alt_u8 *led) {
    acc_read += OFFSET;
    alt_u8 val = (acc_read >> 6) & 0x07;
    *led = (8 >> val) | (8 << (8 - val));
    *level = (acc_read >> 1) & 0x1f;
}

float string_to_float(const char* str) {
    float result = 0.0f;
    float fraction = 0.1f;
    int sign = 1;
    int i = 0;

    // Handle negative numbers
    if (str[0] == '-') {
        sign = -1;
        i++;
    }

    // Process integer part
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10.0f + (str[i] - '0');
        i++;
    }

    // Process fractional part
    if (str[i] == '.') {
        i++;
        while (str[i] >= '0' && str[i] <= '9') {
            result += (str[i] - '0') * fraction;
            fraction *= 0.1f;
            i++;
        }
    }

    return sign * result;
}

void send_accelerometer_data(int num_samples) {
    printf("Starting continuous data transmission...\n");
    for (int i = 0; i <  num_samples || continuous_mode; i++){
        alt_32 x_read, y_read, z_read;
        alt_up_accelerometer_spi_read_x_axis(acc_dev, &x_read);
        alt_up_accelerometer_spi_read_y_axis(acc_dev, &y_read);
        alt_up_accelerometer_spi_read_z_axis(acc_dev, &z_read);
        printf("%ld,%ld,%ld\n", x_read, y_read, z_read);
        usleep(10000); // 10ms delay between samples
    }
    printf("END\n");
    continuous_mode = 0;
}


void uart_isr(void *context, alt_u32 id) {
    static char coeff_buffer[MAX_COEFF_DIGITS];
    static int coeff_index = 0;
    static int tap_index = 0;

    char cmd = IORD_ALTERA_AVALON_JTAG_UART_DATA(JTAG_UART_BASE) & 0xFF;

    if (cmd == '0') {
        mode = 0;
        printf("Mode set to 0 (unfiltered) - Current mode: Unfiltered\n");
    } else if (cmd == '1') {
        mode = 1;
        printf("Mode set to 1 (filtered) - Current mode: Filtered\n");
    } else if (cmd == 'r') {
        alt_32 x_read, y_read, z_read;
        alt_up_accelerometer_spi_read_x_axis(acc_dev, &x_read);
        alt_up_accelerometer_spi_read_y_axis(acc_dev, &y_read);
        alt_up_accelerometer_spi_read_z_axis(acc_dev, &z_read);
        printf("Accelerometer Data: X=%ld, Y=%ld, Z=%ld - Current mode: %s\n", x_read, y_read, z_read, mode ? "Filtered" : "Unfiltered");
    } else if (cmd == 'c') {
        // Start receiving new coefficients
        printf("Ready to receive new coefficients\n");
        coeff_index = 0;
        tap_index = 0;
    } else if (cmd == 'p') {
        printf("Received command: %c\n", cmd);
        continuous_mode = 1;
        send_accelerometer_data(0);
    } else if (cmd == 's') {
        continuous_mode = 0;
        printf("Stopping continuous data transmission\n");
    } else if (tap_index < TAPS) {
        // Receiving coefficient data
        if (cmd == COEFF_DELIMITER || coeff_index == MAX_COEFF_DIGITS - 1) {
            // End of a coefficient
            coeff_buffer[coeff_index] = '\0';
            coeffs[tap_index] = string_to_float(coeff_buffer);
            printf("Received coefficient %d: %f\n", tap_index, coeffs[tap_index]);
            tap_index++;
            coeff_index = 0;
        } else {
            // Add digit to current coefficient
            coeff_buffer[coeff_index++] = cmd;
        }

        if (tap_index == TAPS) {
            printf("All coefficients updated\n");
        }
    }
}
