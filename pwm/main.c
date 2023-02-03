#include "main.h"

#define PPS_PIN 2
#define SIGNAL_PIN 15

volatile int ppsFlag = 0;
volatile absolute_time_t ppsTime;
volatile uint16_t signalCount;
volatile uint32_t signalWrapCount = 0;

uint signalSlice;

void pps_callback(uint gpio, uint32_t events) {
    signalCount = pwm_get_counter(signalSlice);
    ppsTime = get_absolute_time();
    ppsFlag = 1;
}

void on_pwm_wrap() {
    pwm_clear_irq(signalSlice);
    signalWrapCount++;
}

uint setup_pwm_counter(uint gpio) {
    // can only use input on channel B
    assert(pwm_gpio_to_channel(gpio) == PWM_CHAN_B);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_RISING);
    pwm_config_set_clkdiv(&cfg, 1.f);
    pwm_init(slice_num, &cfg, false);
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    // register interrupt handler
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_set_enabled(slice_num, true);
    return slice_num;
}

int main() {
    stdio_init_all();

    printf("Starting...\n");

    // setup signal counter
    signalSlice = setup_pwm_counter(SIGNAL_PIN);

    // setup PPS
    gpio_init(PPS_PIN);
    gpio_set_dir(PPS_PIN, GPIO_IN);
    gpio_pull_down(PPS_PIN);
    gpio_set_irq_enabled_with_callback(PPS_PIN, 0x08, 1, pps_callback);

    uint32_t prevTotal = 0;
    uint32_t seconds = 0;

    while(1) {
        if(ppsFlag == 1) {
            uint32_t millis = to_ms_since_boot(ppsTime);
            uint32_t total = (signalWrapCount * 65535) + signalWrapCount + signalCount;
            uint32_t delta = total - prevTotal;
            printf("%lu %lu %d %d %lu %lu\n", (unsigned long)seconds, (unsigned long)millis, signalWrapCount, signalCount, (unsigned long)total, (unsigned long)delta);
            prevTotal = total;
            seconds++;
            ppsFlag = 0;
        }
        tight_loop_contents();
    }

}